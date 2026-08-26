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

/* FortifyIO.h declares macros to mimic the standard library's stream
   input/output functions in order to redirect function calls to an
   alternative implementation that returns errors if allocations via
   Simon P. Bullen's fortified memory allocation shell fail. This
   allows stress testing.

Dependencies: ANSI C library.
Message tokens: None.
History:
  CJB: 01-Jan-15: Created.
  CJB: 13-Nov-16: Added interceptor versions of fputs and fprintf.
  CJB: 09-Dec-16: Added interceptor versions of fgetc and fputc.
  CJB: 24-Aug-26: Use the _Optional qualifier for referenced types where
                  the pointer can be null.
  CJB: 25-Aug-26: Added portable interception of ferror and clearerr. Intercept
                  every ISO C function that opens a stream.
  CJB: 26-Aug-26: Moved this interface from CBDebugLib to Fortify and renamed
                  its exported functions with the Fortify_ prefix.
*/

#ifndef FortifyIO_h
#define FortifyIO_h

/* ISO library headers */
#include <stdio.h>

#if !defined(USE_OPTIONAL) && !defined(_Optional)
#define _Optional
#endif

#ifdef FORTIFY

#define fopen(filename, mode) \
          Fortify_fopen(filename, mode, __FILE__, __LINE__)

#define freopen(filename, mode, stream) \
          Fortify_freopen(filename, mode, stream, __FILE__, __LINE__)

#define tmpfile() \
          Fortify_tmpfile(__FILE__, __LINE__)

#define rewind(stream) \
          Fortify_rewind(stream, __FILE__, __LINE__)

#define fseek(stream, offset, whence) \
          Fortify_fseek(stream, offset, whence, __FILE__, __LINE__)

#define ftell(stream) \
          Fortify_ftell(stream, __FILE__, __LINE__)

#define fclose(stream) \
          Fortify_fclose(stream, __FILE__, __LINE__)

#define fwrite(ptr, size, nmemb, stream) \
          Fortify_fwrite(ptr, size, nmemb, stream, __FILE__, __LINE__)

#define fread(ptr, size, nmemb, stream) \
          Fortify_fread(ptr, size, nmemb, stream, __FILE__, __LINE__)

#define fputs(s, stream) \
          Fortify_fputs(s, stream, __FILE__, __LINE__)

#define puts(s) \
          Fortify_puts(s, __FILE__, __LINE__)

#define fprintf(stream, ...) \
          Fortify_fprintf(stream, __FILE__, __LINE__, __VA_ARGS__)

#define fgetc(stream) \
          Fortify_fgetc(stream, __FILE__, __LINE__)

#define fputc(c, stream) \
          Fortify_fputc(c, stream, __FILE__, __LINE__)

#define ferror(stream) \
          Fortify_ferror(stream)

#define clearerr(stream) \
          Fortify_clearerr(stream)

#endif

#ifdef __cplusplus
extern "C" {
#endif

_Optional FILE *Fortify_fopen(const char *filename, const char *mode,
                              const char *file, unsigned long line);

_Optional FILE *Fortify_freopen(const char *filename, const char *mode,
                                FILE *stream, const char *file,
                                unsigned long line);

_Optional FILE *Fortify_tmpfile(const char *file, unsigned long line);

void Fortify_rewind(FILE *stream, const char *file, unsigned long line);

int Fortify_fseek(FILE *stream, long offset, int whence, const char *file,
                  unsigned long line);

long Fortify_ftell(FILE *stream, const char *file, unsigned long line);

int Fortify_fclose(FILE *stream, const char *file, unsigned long line);

size_t Fortify_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream,
                      const char *file, unsigned long line);

size_t Fortify_fread(void *ptr, size_t size, size_t nmemb, FILE *stream,
                     const char *file, unsigned long line);

int Fortify_fputs(const char *s, FILE *stream, const char *file,
                  unsigned long line);

int Fortify_puts(const char *s, const char *file, unsigned long line);

int Fortify_fprintf(FILE *stream, const char *file, unsigned long line,
                    const char *format, ...);

int Fortify_fgetc(FILE *stream, const char *file, unsigned long line);

int Fortify_fputc(int c, FILE *stream, const char *file, unsigned long line);

int Fortify_ferror(FILE *stream);

void Fortify_clearerr(FILE *stream);

#ifdef __cplusplus
}
#endif

#endif
