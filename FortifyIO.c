/*
 * Fortify: Error injection veneer to standard library I/O functions
 * Copyright (C) 2015 Christopher Bazley
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* History:
  CJB: 01-Jan-15: Created this source file.
  CJB: 18-Apr-15: Assertions are now provided by debug.h.
  CJB: 13-Nov-16: Added interceptor versions of fputs and fprintf.
  CJB: 09-Dec-16: Added interceptor versions of fgetc and fputc.
  CJB: 13-Jun-20: Use new Fortify_AllowAllocate to avoid accumulating huge
                  numbers of 'freed' dummy memory allocations.
  CJB: 24-Aug-26: Use the _Optional qualifier for referenced types where
                  the pointer can be null.
  CJB: 25-Aug-26: Track simulated stream errors without accessing non-standard
                  members of FILE. Intercept every ISO C function that opens
                  a stream.
  CJB: 26-Aug-26: Moved this implementation from CBDebugLib to Fortify,
                  removed its dependency on CBDebugLib headers and renamed
                  its exported functions with the Fortify_ prefix.
*/

#undef FORTIFY /* Prevent macro redirection of IO function calls to
                  Fortify_... functions within this source file. */

/* ISO library headers */
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

/* Local headers */
#include "FortifyIO.h"
#include "fortify.h"

typedef struct StreamState
{
  _Optional struct StreamState *next;
  FILE *stream;
  bool error;
  bool allocated;
} StreamState;

static _Optional StreamState *stream_states;
static StreamState standard_stream_states[3];
static bool stream_states_initialized;

static void init_stream_states(void)
{
  if (!stream_states_initialized)
  {
    standard_stream_states[0] =
      (StreamState){&standard_stream_states[1], stdin, false, false};
    standard_stream_states[1] =
      (StreamState){&standard_stream_states[2], stdout, false, false};
    standard_stream_states[2] =
      (StreamState){NULL, stderr, false, false};
    stream_states = &standard_stream_states[0];
    stream_states_initialized = true;
  }
}

static _Optional StreamState *find_stream_state(FILE *const stream)
{
  assert(stream);
  init_stream_states();

  for (_Optional StreamState *state = stream_states; state != NULL;
       state = state->next)
  {
    if (state->stream == stream)
    {
      return state;
    }
  }

  return NULL;
}

static bool stream_state_is_member(const StreamState *const state)
{
  assert(state);
  init_stream_states();

  for (_Optional StreamState *item = stream_states; item != NULL;
       item = item->next)
  {
    if (&*item == state)
    {
      return true;
    }
  }

  return false;
}

static StreamState *get_stream_state(FILE *const stream)
{
  _Optional StreamState *const state = find_stream_state(stream);

  assert(state != NULL);
  assert(stream_state_is_member(&*state));
  return &*state;
}

static bool add_stream(FILE *const stream)
{
  assert(stream);
  init_stream_states();
  assert(find_stream_state(stream) == NULL);

  _Optional StreamState *const state = malloc(sizeof *state);
  if (state == NULL)
  {
    return false;
  }

  *state = (StreamState){stream_states, stream, false, true};
  stream_states = state;
  assert(stream_state_is_member(&*state));
  return true;
}

static void remove_stream(FILE *const stream)
{
  assert(stream);
  init_stream_states();
  _Optional StreamState **link = &stream_states;

  while (*link != NULL && (*link)->stream != stream)
  {
    link = &(*link)->next;
  }

  assert(*link != NULL);
  StreamState *const state = &**link;
  assert(stream_state_is_member(state));
  *link = state->next;
  if (state->allocated)
  {
    free(state);
  }
}

static bool io_succeeds(const char *file, unsigned long line)
{
  /* CJB's extra Fortify function to avoid accumulating
     huge numbers of 'freed' dummy memory allocations. */
  return Fortify_AllowAllocate(file, line);
}

_Optional FILE *Fortify_fopen(const char *filename, const char *mode,
                              const char *file, unsigned long line)
{
  _Optional FILE *fh;
  assert(filename);
  assert(mode);
  if (io_succeeds(file, line))
  {
    fh = fopen(filename, mode);
    if (fh != NULL && !add_stream(&*fh))
    {
      fclose(&*fh);
      errno = ENOMEM;
      fh = NULL;
    }
  }
  else
  {
    errno = ERANGE;
    fh = NULL;
  }
  return fh;
}

_Optional FILE *Fortify_freopen(const char *filename, const char *mode,
                                FILE *stream, const char *file,
                                unsigned long line)
{
  _Optional FILE *fh;
  assert(filename);
  assert(mode);
  assert(stream);
  (void)get_stream_state(stream);

  /* freopen closes the old stream regardless of whether opening the new
     stream succeeds, so its state record cannot be retained. */
  remove_stream(stream);
  if (io_succeeds(file, line))
  {
    fh = freopen(filename, mode, stream);
  }
  else
  {
    (void)fclose(stream);
    errno = ERANGE;
    fh = NULL;
  }

  if (fh != NULL && !add_stream(&*fh))
  {
    fclose(&*fh);
    errno = ENOMEM;
    fh = NULL;
  }
  return fh;
}

_Optional FILE *Fortify_tmpfile(const char *file, unsigned long line)
{
  _Optional FILE *fh;

  if (io_succeeds(file, line))
  {
    fh = tmpfile();
    if (fh != NULL && !add_stream(&*fh))
    {
      fclose(&*fh);
      errno = ENOMEM;
      fh = NULL;
    }
  }
  else
  {
    errno = ERANGE;
    fh = NULL;
  }
  return fh;
}

void Fortify_rewind(FILE *stream, const char *file, unsigned long line)
{
  assert(stream);
  StreamState *const state = get_stream_state(stream);
  if (io_succeeds(file, line))
  {
    rewind(stream);
    state->error = false;
  }
  else
  {
    errno = ERANGE;
  }
}

int Fortify_fseek(FILE *stream, long offset, int whence, const char *file,
                  unsigned long line)
{
  int err;
  assert(stream);
  (void)get_stream_state(stream);
  assert(whence == SEEK_SET || whence == SEEK_CUR || whence == SEEK_END);
  if (io_succeeds(file, line))
  {
    err = fseek(stream, offset, whence);
  }
  else
  {
    errno = ERANGE;
    err = -1;
  }
  return err;
}

long Fortify_ftell(FILE *stream, const char *file, unsigned long line)
{
  long fpos;
  assert(stream);
  (void)get_stream_state(stream);
  if (io_succeeds(file, line))
  {
    fpos = ftell(stream);
  }
  else
  {
    errno = ERANGE;
    fpos = -1;
  }
  return fpos;
}

int Fortify_fclose(FILE *stream, const char *file, unsigned long line)
{
  int err;
  assert(stream);
  (void)get_stream_state(stream);
  /* Close the file even if simulating failure, to prevent leakage of
     file handles. */
  remove_stream(stream);
  err = fclose(stream);
  if (!io_succeeds(file, line))
  {
    errno = ERANGE;
    err = EOF;
  }
  return err;
}

size_t Fortify_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream,
                      const char *file, unsigned long line)
{
  size_t nwritten;
  assert(ptr);
  assert(stream);
  StreamState *const state = get_stream_state(stream);
  if ((stream == stderr) || io_succeeds(file, line))
  {
    nwritten = fwrite(ptr, size, nmemb, stream);
  }
  else
  {
    state->error = true;
    errno = ERANGE;
    nwritten = 0;
  }
  return nwritten;
}

size_t Fortify_fread(void *ptr, size_t size, size_t nmemb, FILE *stream,
                     const char *file, unsigned long line)
{
  size_t nread;
  assert(ptr);
  assert(stream);
  StreamState *const state = get_stream_state(stream);
  if (io_succeeds(file, line))
  {
    nread = fread(ptr, size, nmemb, stream);
  }
  else
  {
    state->error = true;
    errno = ERANGE;
    nread = 0;
  }
  return nread;
}

int Fortify_fputs(const char *s, FILE *stream, const char *file,
                  unsigned long line)
{
  int err;
  assert(s);
  assert(stream);
  StreamState *const state = get_stream_state(stream);
  if ((stream == stderr) || io_succeeds(file, line))
  {
    err = fputs(s, stream);
  }
  else
  {
    state->error = true;
    errno = ERANGE;
    err = EOF;
  }
  return err;
}

int Fortify_puts(const char *s, const char *file, unsigned long line)
{
  int err;
  assert(s);
  StreamState *const state = get_stream_state(stdout);
  if (io_succeeds(file, line))
  {
    err = puts(s);
  }
  else
  {
    state->error = true;
    errno = ERANGE;
    err = EOF;
  }
  return err;
}

int Fortify_fprintf(FILE *stream, const char *file, unsigned long line,
                    const char *format, ...)
{
  int nchars;
  assert(stream);
  assert(format);
  StreamState *const state = get_stream_state(stream);

  if ((stream == stderr) || io_succeeds(file, line))
  {
    va_list arg;
    va_start(arg, format);
    nchars = vfprintf(stream, format, arg);
    va_end(arg);
  }
  else
  {
    state->error = true;
    errno = ERANGE;
    nchars = -1;
  }
  return nchars;
}

int Fortify_fgetc(FILE *stream, const char *file, unsigned long line)
{
  int c;
  assert(stream);
  StreamState *const state = get_stream_state(stream);
  if (io_succeeds(file, line))
  {
    c = fgetc(stream);
  }
  else
  {
    state->error = true;
    errno = ERANGE;
    c = EOF;
  }
  return c;
}

int Fortify_fputc(int c, FILE *stream, const char *file, unsigned long line)
{
  int err;
  assert(stream);
  StreamState *const state = get_stream_state(stream);
  if ((stream == stderr) || io_succeeds(file, line))
  {
    err = fputc(c, stream);
  }
  else
  {
    state->error = true;
    errno = ERANGE;
    err = EOF;
  }
  return err;
}

int Fortify_ferror(FILE *const stream)
{
  assert(stream);
  const StreamState *const state = get_stream_state(stream);
  return state->error || ferror(stream);
}

void Fortify_clearerr(FILE *const stream)
{
  assert(stream);
  StreamState *const state = get_stream_state(stream);
  clearerr(stream);
  state->error = false;
}
