/*  Stereo Enhancer XMMS plugin
 *
 *  Copyright (C) 2006-2010 Teru KAMOGASHIRA
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "fv3_config.h"
#include "libxmmsplugin.hpp"
#include <freeverb/stenh.hpp>
#include <freeverb/slot.hpp>
#include <freeverb/fv3_ch_tool.hpp>
#include <libsamplerate2/samplerate2.h>

static const char *about_text =
  "Freeverb3 "VERSION"\n"
  "Stereo Enhancer (v3F)\n"
  "XMMS/BMP/Audacious Plugin\n"
#ifdef PLUGDOUBLE
  "Double Precision Version\n"
#else
  "Single Precision Version\n"
#endif
  "Copyright (C) 2006-2010 Teru KAMOGASHIRA\n"
  "http://freeverb3.sourceforge.net/";
static const char * productString = "Freeverb3 "VERSION" [StereoEnhancer(v3F)]";
static const char * configSectionString = "freeverb3_plugin_stenh";

#ifdef PLUGDOUBLE
static fv3::stenh_ DSP;
typedef fv3::slot_ SLOTP;
#else
static fv3::stenh_f DSP;
typedef fv3::slot_f SLOTP;
#endif

static int currentfs = 0;
static pthread_mutex_t plugin_mutex;
static gboolean plugin_available = false;
static fv3::libxmmsplugin *XMMSPlugin = NULL;

static void chvall(pfloat_t t){DSP.setChValL(t);};
static void chvalr(pfloat_t t){DSP.setChValL(t);};
static void bpflpf(pfloat_t t){DSP.setBPF_LPF(t);};
static void bpfhpf(pfloat_t t){DSP.setBPF_HPF(t);};
static void brflpf(pfloat_t t){DSP.setBRF_LPF(t);};
static void brfhpf(pfloat_t t){DSP.setBRF_HPF(t);};
static void _width(pfloat_t t){DSP.setWidth(t);};
static void _dry(pfloat_t t){DSP.setDry(t);};
static void _diffusion(pfloat_t t){DSP.setDiffusion(t);};
static void _threshold(pfloat_t t){DSP.setThreshold(t);};
static void _rms(pfloat_t t){DSP.setRMS(t);};
static void _attack(pfloat_t t){DSP.setAttack(t);};
static void _release(pfloat_t t){DSP.setRelease(t);};
static void _softknee(pfloat_t t){DSP.setSoftKnee(t);};
static void _ratio(pfloat_t t){DSP.setRatio(t);};
static void _depth1(pfloat_t t){DSP.setBPFDepth(t);};
static void _depth2(pfloat_t t){DSP.setBRFDepth(t);};
static void _depth3(pfloat_t t){DSP.setOverallDepth(t);};

// configurations
static PluginParameterTable ppConfTable[] = {
  {"ChValL","ChValL","", "",            0.0,1.0,2,    1, 0,true,true,false,kFloat,(void*)chvall,},
  {"ChValR","ChValR","", "",            0.0,1.0,2,    1, 0,true,true,false,kFloat,(void*)chvalr,},
  {"BPF_LPF","BPF_LPF","", "",          0.0,1.0,2,  0.5, 0,true,true,false,kFloat,(void*)bpflpf,},
  {"BPF_HPF","BPF_HPF","", "",          0.0,1.0,2,  0.5, 0,true,true,false,kFloat,(void*)bpfhpf,},
  {"BRF_LPF","BRF_LPF","", "",          0.0,1.0,2,  0.5, 0,true,true,false,kFloat,(void*)brflpf,},
  {"BRF_HPF","BRF_HPF","", "",          0.0,1.0,2,  0.5, 0,true,true,false,kFloat,(void*)brfhpf,},
  {"Width","Width","", "",              0.0,4.0,2,  0.7, 0,true,true,false,kFloat,(void*)_width,},
  {"Dry","Dry","", "",                  0.0,4.0,2,  1.0, 0,true,true,false,kFloat,(void*)_dry,},
  {"Diffusion","Diffusion","", "",      0.0,3.0,2,  0.9, 0,true,true,false,kFloat,(void*)_diffusion,},
  {"Threshold","Threshold","[dB]", "",-90.0,10.0,2,  -1, 0,true,true,false,kFloat,(void*)_threshold,},
  {"RMS","RMS","[ms]", "",              0.0,100.0,2,  0, 0,true,false,false,kFloat,(void*)_rms,},
  {"Attack","Attack","[ms]", "",        0.0,10000.0,2,0, 0,true,true,false,kFloat,(void*)_attack,},
  {"Release","Release","[ms]", "",      0.0,10000.0,2,0, 0,true,true,false,kFloat,(void*)_release,},
  {"SoftKnee","SoftKnee","[dB]", "",    0.0,20.0,2,  10, 0,true,true,false,kFloat,(void*)_softknee,},
  {"Ratio","Ratio","[X]", "",           0.1,20.0,2,   3, 0,true,true,false,kFloat,(void*)_ratio,},
  {"BPF Depth","Depth1","[ms]", "",     0.0,50.0,2,   4, 0,true,false,false,kFloat,(void*)_depth1,},
  {"BRF Depth","Depth2","[ms]", "",     0.0,50.0,2,   7, 0,true,false,false,kFloat,(void*)_depth2,},
  {"Overall Depth","Depth3","[ms]", "", 0.0,50.0,2,   7, 0,true,false,false,kFloat,(void*)_depth3,},
};

static void
#ifdef __GNUC__
__attribute__ ((constructor))
#endif
plugin_init(void)
{
  std::fprintf(stderr, "stenh.cpp: plugin_init()\n");
  pthread_mutex_init(&plugin_mutex, NULL);
}

static void
#ifdef __GNUC__
__attribute__ ((destructor))
#endif
plugin_fini(void)
{
  std::fprintf(stderr, "stenh.cpp: plugin_fini()\n");
  pthread_mutex_destroy(&plugin_mutex);
}

static void mod_samples(pfloat_t * iL, pfloat_t * iR, pfloat_t * oL, pfloat_t * oR, gint length, gint srate)
{
  if(pthread_mutex_trylock(&plugin_mutex) == EBUSY) return;
  if(plugin_available != true||XMMSPlugin == NULL)
    {
      pthread_mutex_unlock(&plugin_mutex);
      return;
    }
  if(currentfs != srate)
    {
      fprintf(stderr, "stenh.cpp: Fs %d -> %d, resetAll\n", currentfs, srate);
      currentfs = srate;
      DSP.setCurrentFs(srate);
    }
  XMMSPlugin->callNRTParameters();
  DSP.processreplace(iL,iR,oL,oR,length);
  pthread_mutex_unlock(&plugin_mutex);
}

static void init(void)
{
#ifndef __GNUC__
  plugin_init();
#endif
  fprintf(stderr, "stenh.cpp: init()\n");
  pthread_mutex_lock(&plugin_mutex);
  plugin_available = true;
  if(XMMSPlugin != NULL) delete XMMSPlugin;
  XMMSPlugin = new fv3::libxmmsplugin(ppConfTable, sizeof(ppConfTable)/sizeof(PluginParameterTable),
				      about_text, productString, configSectionString);  
  XMMSPlugin->registerModSamples(mod_samples);
  currentfs = 0;
  pthread_mutex_unlock(&plugin_mutex);
}

static void cleanup(void)
{
  fprintf(stderr, "stenh.cpp: cleanup()\n");
  pthread_mutex_lock(&plugin_mutex);
  plugin_available = false;
  fprintf(stderr, "stenh.cpp: WARNING: cleanup during play is not supported!!\n");
  delete XMMSPlugin;
  XMMSPlugin = NULL;
  pthread_mutex_unlock(&plugin_mutex);
#ifndef __GNUC__
  plugin_fini();
#endif
}

static void configure(void){if(XMMSPlugin != NULL)XMMSPlugin->configure();}
static void about(void){if(XMMSPlugin != NULL)XMMSPlugin->about();}
#if __AUDACIOUS_PLUGIN_API__ >= 13
static gint decoder_to_output_time(gint time){return time;}
static gint output_to_decoder_time(gint time){return time;}
static void stenh_start(gint * channels, gint * rate){if(XMMSPlugin != NULL) XMMSPlugin->start(channels, rate);}
static void stenh_process(gfloat ** data, gint * samples){if(XMMSPlugin != NULL) XMMSPlugin->process(data,samples);}
static void stenh_flush(){if(XMMSPlugin != NULL) XMMSPlugin->flush();}
static void stenh_finish(gfloat ** data, gint * samples){if(XMMSPlugin != NULL) XMMSPlugin->finish(data,samples);}
#else
static void query_format(AFormat * fmt, gint * rate, gint * nch)
{if(XMMSPlugin != NULL)XMMSPlugin->query_format(fmt,rate,nch);}
static int  mod_samples(gpointer * d, gint length, AFormat afmt, gint srate, gint nch)
{if(XMMSPlugin != NULL) return XMMSPlugin->mod_samples(d,length,afmt,srate,nch); return 0;}
#endif

#ifndef AUDACIOUS140
static EffectPlugin ep = {
  NULL, NULL, (char*)productString,
  init, cleanup, about, configure, mod_samples, query_format,
};
extern "C" EffectPlugin *get_eplugin_info(void)
{
#ifdef DEBUG
  fprintf(stderr, "freeverb3.cpp: get_eplugin_info()\n");
#endif
  init();
  return &ep;
}

#else
static EffectPlugin epe = {
  NULL, /* handle */
  NULL, /* filename */
  (gchar*)productString, /* description */
  init,
  cleanup,
  about,
  configure,

#if __AUDACIOUS_PLUGIN_API__ >= 10
  NULL, /* settings */
#endif

#if __AUDACIOUS_PLUGIN_API__ >= 16
  NULL, /* sendmsg */
#endif

#if __AUDACIOUS_PLUGIN_API__ < 16
  FALSE, /* enabled */
#endif

#if __AUDACIOUS_PLUGIN_API__ < 16
# if __AUDACIOUS_PLUGIN_API__ >= 13
  NULL, NULL, /* mod_samples, query_format */
# else
  mod_samples, query_format,
# endif
#endif

#if __AUDACIOUS_PLUGIN_API__ >= 13
  stenh_start,
  stenh_process,
  stenh_flush,
  stenh_finish,
  decoder_to_output_time,
  output_to_decoder_time,
#endif

#if __AUDACIOUS_PLUGIN_API__ >= 16
  0, /* order */
  TRUE, /* preserves_format */
#endif
};

static EffectPlugin *eplist[] = {&epe, NULL};
SIMPLE_EFFECT_PLUGIN(plugin, eplist);
#endif
