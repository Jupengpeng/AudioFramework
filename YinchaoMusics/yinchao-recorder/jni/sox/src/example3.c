/* Simple example of using SoX libraries
 *
 * Copyright (c) 2007-9 robs@users.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef NDEBUG /* N.B. assert used with active statements so enable always. */
#undef NDEBUG /* Must undef above assert.h or other that might include it. */
#endif

#include "sox.h"
#include "util.h"
#include <stdio.h>
#include <assert.h>

/*
 * Example of a custom output message handler.
 */
static void output_message(unsigned level, const char *filename, const char *fmt, va_list ap)
{
  char const * const str[] = {"FAIL", "WARN", "INFO", "DBUG"};
  if (sox_globals.verbosity >= level) {
    fprintf(stderr, "%s ", str[min(level - 1, 3)]);
    sox_output_message(stderr, filename, fmt, ap);
    fprintf(stderr, "\n");
  }
}

/*
 * On an alsa capable system, plays an audio file starting 10 seconds in.
 * Copes with sample-rate and channel change if necessary since its
 * common for audio drivers to to support subset of rates and channel
 * counts..
 * E.g. example3 song2.ogg
 *
 * Can easily be changed to work with other audio device drivers supported
 * by libSoX; e.g. "oss", "ao", "coreaudio", etc.
 * See the soxformat(7) manual page.
 */
int main(int argc, char * argv[])
{
  static sox_format_t * in, * out; /* input and output files */
  sox_effects_chain_t * chain;
  sox_effect_t * e;
  char * args[10];

  assert(argc == 2);
  sox_globals.output_message_handler = output_message;
  sox_globals.verbosity = 1;

  assert(sox_init() == SOX_SUCCESS);
  assert(in = sox_open_read(argv[1], NULL, NULL, NULL));
  /* Change "alsa" in this line to use an alternative audio device driver: */
  assert(out= sox_open_write("default", &in->signal, NULL, "alsa", NULL, NULL));

  chain = sox_create_effects_chain(&in->encoding, &out->encoding);

  e = sox_create_effect(sox_find_effect("input"));
  args[0] = (char *)in, assert(sox_effect_options(e, 1, args) == SOX_SUCCESS);
  assert(sox_add_effect(chain, e, &in->signal, &in->signal) == SOX_SUCCESS);

  e = sox_create_effect(sox_find_effect("trim"));
  args[0] = "10", assert(sox_effect_options(e, 1, args) == SOX_SUCCESS);
  assert(sox_add_effect(chain, e, &in->signal, &in->signal) == SOX_SUCCESS);

  if (in->signal.rate != out->signal.rate) {
    e = sox_create_effect(sox_find_effect("rate"));
    assert(sox_effect_options(e, 0, NULL) == SOX_SUCCESS);
    assert(sox_add_effect(chain, e, &in->signal, &out->signal) == SOX_SUCCESS);
  }

  if (in->signal.channels != out->signal.channels) {
    e = sox_create_effect(sox_find_effect("channels"));
    assert(sox_effect_options(e, 0, NULL) == SOX_SUCCESS);
    assert(sox_add_effect(chain, e, &in->signal, &out->signal) == SOX_SUCCESS);
  }

  e = sox_create_effect(sox_find_effect("output"));
  args[0] = (char *)out, assert(sox_effect_options(e, 1, args) == SOX_SUCCESS);
  assert(sox_add_effect(chain, e, &in->signal, &out->signal) == SOX_SUCCESS);

  sox_flow_effects(chain, NULL, NULL);

  sox_delete_effects_chain(chain);
  sox_close(out);
  sox_close(in);
  sox_quit();

  return 0;
}
