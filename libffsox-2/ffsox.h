/*
 * ffsox.h
 * Copyright (C) 2014 Peter Belkner <pbelkner@users.sf.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */
#ifndef __FFSOX_H__
#define __FFSOX_H__ // [
#if defined (_WIN32) // [
#include <mmdeviceapi.h>
#include <audioclient.h>
#endif // ]
#include <lib1770.h>
#include <ffsox_dynload.h>
#ifdef __cpluplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
#if defined (_WIN32) // {
#define FFSOX_REFTIMES_PER_SEC \
    10000000
#define FFSOX_REFTIMES_PER_MILLISEC \
    10000
#endif // }

#define FFSOX_LFE_CHANNEL 3
#define FFSOX_SAMPLE_FMT_S24 AV_SAMPLE_FMT_NB

/*
 * #881132 seems to be an artifical test case providing audio with a number of
 * channels (32) greater than FFmpeg can deal with (AV_NUM_DATA_POINTERS==8).
 */
#define FFSOX_FIX_881131_CHANNEL_UNDERFOW
#define FFSOX_FIX_881132_CHANNEL_OVERFLOW
#define FFSOX_FIX_883198_MISSING_CODEC_ID

///////////////////////////////////////////////////////////////////////////////
typedef struct ffsox_intercept ffsox_intercept_t;
typedef struct ffsox_convert ffsox_convert_t;
typedef struct ffsox_format ffsox_format_t;
typedef struct ffsox_stream ffsox_stream_t;
typedef struct ffsox_frame ffsox_frame_t;
typedef struct ffsox_engine ffsox_engine_t;
typedef struct ffsox_read_list ffsox_read_list_t;
typedef struct ffsox_packet_consumer_list ffsox_packet_consumer_list_t;
typedef struct ffsox_stream_list ffsox_stream_list_t;
typedef struct ffsox_sink ffsox_sink_t;

typedef struct ffsox_aggregate ffsox_aggregate_t;
typedef struct ffsox_collect ffsox_collect_t;

typedef struct ffsox_block_config ffsox_block_config_t;
typedef struct ffsox_collect_config ffsox_collect_config_t;
typedef struct ffsox_analyze_config ffsox_analyze_config_t;

///////////////////////////////////////////////////////////////////////////////
typedef struct ffsox_node_vmt ffsox_node_vmt_t;
  typedef struct ffsox_source_vmt ffsox_source_vmt_t;
  typedef struct ffsox_packet_consumer_vmt ffsox_packet_consumer_vmt_t;
    typedef struct ffsox_packet_writer_vmt ffsox_packet_writer_vmt_t;
    typedef struct ffsox_frame_reader_vmt ffsox_frame_reader_vmt_t;
  typedef struct ffsox_frame_consumer_vmt ffsox_frame_consumer_vmt_t;
    typedef struct ffsox_frame_writer_vmt ffsox_frame_writer_vmt_t;
#if defined (_WIN32) // {
    typedef struct ffsox_audio_player_vmt ffsox_audio_player_vmt_t;
#endif // }
    typedef struct ffsox_sox_reader_vmt ffsox_sox_reader_vmt_t;

typedef struct ffsox_node ffsox_node_t;
  typedef struct ffsox_source ffsox_source_t;
  typedef struct ffsox_packet_consumer ffsox_packet_consumer_t;
    typedef struct ffsox_packet_writer ffsox_packet_writer_t;
    typedef struct ffsox_frame_reader ffsox_frame_reader_t;
  typedef struct ffsox_frame_consumer ffsox_frame_consumer_t;
    typedef struct ffsox_frame_writer ffsox_frame_writer_t;
#if defined (_WIN32) // {
    typedef struct ffsox_audio_player ffsox_audio_player_t;
#endif // }
    typedef struct ffsox_sox_reader ffsox_sox_reader_t;

/// utilities /////////////////////////////////////////////////////////////////
typedef void (*ffsox_pull_callback_t)(void *, double x);

sox_effect_handler_t const *ffsox_sox_read_handler(void);
sox_effect_handler_t const *ffsox_sox_pull_handler(void);

int ffsox_sox_add_effect(sox_effect_t *e, sox_effects_chain_t *chain,
    sox_signalinfo_t *signal_in, sox_signalinfo_t const *signal_out,
    int n, char *opts[]);
int ffsox_sox_add_effect_fn(sox_effects_chain_t *chain,
    sox_signalinfo_t *signal_in, sox_signalinfo_t const *signal_out,
    int n, char *opts[], sox_effect_fn_t fn);
int ffsox_sox_add_effect_name(sox_effects_chain_t *chain,
    sox_signalinfo_t *signal_in, sox_signalinfo_t const *signal_out,
    int n, char *opts[], const char *name);

#if defined (_WIN32) // {
wchar_t *ffsox_path3sep(const wchar_t *ws1, const char *s2, const char *s3,
    wchar_t sep1, wchar_t sep2);
wchar_t *ffsox_path3(const wchar_t *ws1, const char *s2, const char *s3);
#else // } {
char *ffsox_path3sep(const char *s1, const char *s2, const char *s3,
    int sep1, int sep2);
char *ffsox_path3(const char *s1, const char *s2, const char *s3);
#endif // }
int ffsox_csv2avdict(const char *file, char sep, AVDictionary **metadata);

AVCodec *ffsox_find_decoder(enum AVCodecID id);
int ffsox_audiostream(AVFormatContext *fci, int *aip, int *vip);

#if defined (_MSC_VER) // {
extern const AVRational AV_TIME_BASE_Q;
#endif // }

/// intercept /////////////////////////////////////////////////////////////////
struct ffsox_intercept {
  void *data;
  void (*channel)(void *data, int ch, double x);
  void (*sample)(void *data);
};

/// convert ///////////////////////////////////////////////////////////////////
struct ffsox_convert {
  ffsox_frame_t *fr;
  ffsox_frame_t *fw;
  double q;
  ffsox_intercept_t *intercept;
  int channels;
  int nb_samples;
};

#if defined (FFSOX_FIX_881132_CHANNEL_OVERFLOW) // [
int ffsox_convert_setup(ffsox_convert_t *convert, ffsox_frame_t *fr,
    ffsox_frame_t *fw, double q, ffsox_intercept_t *intercept);
#else // ] [
void ffsox_convert_setup(ffsox_convert_t *convert, ffsox_frame_t *fr,
    ffsox_frame_t *fw, double q, ffsox_intercept_t *intercept);
#endif // ]

/// format ////////////////////////////////////////////////////////////////////
struct ffsox_format {
  const char *path;
  AVFormatContext *fc;
};

/// stream ////////////////////////////////////////////////////////////////////
struct ffsox_stream {
  AVFormatContext *fc;
  int stream_index;
  AVStream *st;
  AVCodecContext *cc;
  const AVCodec *codec;
};

int ffsox_stream_new(ffsox_stream_t *s, ffsox_sink_t *so, AVCodec *codec);
int ffsox_stream_interleaved_write(ffsox_stream_t *s, AVPacket *pkt);

/// frame /////////////////////////////////////////////////////////////////////
struct ffsox_frame {
  AVFrame *frame;

  struct {
    int frame;
    int64_t stream;
  } nb_samples;
};

int ffsox_frame_create(ffsox_frame_t *f);
int ffsox_frame_create_cc(ffsox_frame_t *f, AVCodecContext *cc);
void ffsox_frame_cleanup(ffsox_frame_t *f);

int ffsox_frame_complete(ffsox_frame_t *f);
void ffsox_frame_reset(ffsox_frame_t *f);
int ffsox_frame_convert(ffsox_frame_t *fr, ffsox_frame_t *fw, double q);
int ffsox_frame_convert_sox(ffsox_frame_t *fr, ffsox_frame_t *fw, double q,
    ffsox_intercept_t *intercept, sox_uint64_t *clipsp);

/// engine ////////////////////////////////////////////////////////////////////
#define FFSOX_MACHINE_PUSH   0
#define FFSOX_MACHINE_POP    1

struct ffsox_engine {
  ffsox_source_t *source;
  ffsox_node_t *node;
};

int ffsox_engine_run(ffsox_engine_t *e, ffsox_node_t *node);

/// stream_list ///////////////////////////////////////////////////////////////
struct ffsox_stream_list {
#define FFSOX_STREAM_LIST_MEM(T) \
  PBU_LIST_MEM(T) \
  ffsox_stream_t *si; \
  ffsox_stream_t *so;
  FFSOX_STREAM_LIST_MEM(ffsox_stream_list_t)
};

/// packet_consumer_list //////////////////////////////////////////////////////
struct ffsox_packet_consumer_list {
#define FFSOX_PACKET_CONSUMER_LIST_MEM(T) \
  PBU_LIST_MEM(T) \
  ffsox_packet_consumer_t *consumer;
  FFSOX_PACKET_CONSUMER_LIST_MEM(ffsox_packet_consumer_list_t)
};

void ffsox_packet_consumer_list_free(ffsox_packet_consumer_list_t *n);

//// sink /////////////////////////////////////////////////////////////////////
struct ffsox_sink {
  ffsox_format_t f;
  ffsox_stream_list_t *streams;
};

int ffsox_sink_create(ffsox_sink_t *s, const char *path);
void ffsox_sink_cleanup(ffsox_sink_t *s);

int ffsox_sink_append(ffsox_sink_t *sink, ffsox_stream_t *si,
    ffsox_stream_t *so);

int ffsox_sink_open(ffsox_sink_t *s);
void ffsox_sink_close(ffsox_sink_t *s);

///////////////////////////////////////////////////////////////////////////////
enum {
  FFSOX_AGGREGATE_MOMENTARY_MAXIMUM=1<<0,
  FFSOX_AGGREGATE_MOMENTARY_MEAN=1<<1,
  FFSOX_AGGREGATE_MOMENTARY_RANGE=1<<2,
  FFSOX_AGGREGATE_SHORTTERM_MAXIMUM=1<<3,
  FFSOX_AGGREGATE_SHORTTERM_MEAN=1<<4,
  FFSOX_AGGREGATE_SHORTTERM_RANGE=1<<5,
  FFSOX_AGGREGATE_SAMPLEPEAK=1<<6,
  FFSOX_AGGREGATE_TRUEPEAK=1<<7,
  FFSOX_AGGREGATE_MOMENTARY
      =FFSOX_AGGREGATE_MOMENTARY_MAXIMUM
      |FFSOX_AGGREGATE_MOMENTARY_MEAN
      |FFSOX_AGGREGATE_MOMENTARY_RANGE,
  FFSOX_AGGREGATE_SHORTTERM
      =FFSOX_AGGREGATE_SHORTTERM_MAXIMUM
      |FFSOX_AGGREGATE_SHORTTERM_MEAN
      |FFSOX_AGGREGATE_SHORTTERM_RANGE,
  FFSOX_AGGREGATE_ALL
      =FFSOX_AGGREGATE_MOMENTARY
      |FFSOX_AGGREGATE_SHORTTERM
      |FFSOX_AGGREGATE_SAMPLEPEAK
      |FFSOX_AGGREGATE_TRUEPEAK
};

struct ffsox_aggregate {
  int flags;
  double samplepeak;
  double truepeak;
  lib1770_stats_t *momentary;
  lib1770_stats_t *shortterm;
};

int ffsox_aggregate_create(ffsox_aggregate_t *aggregate, int flags);
void ffsox_aggregate_cleanup(ffsox_aggregate_t *aggregate);

int ffsox_aggregate_merge(ffsox_aggregate_t *lhs, ffsox_aggregate_t *rhs);

///////////////////////////////////////////////////////////////////////////////
struct ffsox_block_config {
  double ms;
  int partition;
};

struct ffsox_collect_config {
  ffsox_aggregate_t *aggregate;
  double scale;
  double samplerate;
  int channels;
#if defined (FFSOX_LFE_CHANNEL) // [
  int lfe;
#endif // ]
  ffsox_block_config_t momentary;
  ffsox_block_config_t shortterm;
};

struct ffsox_collect {
  ffsox_aggregate_t *aggregate;
  double scale;
  double invscale;
  int channels;
  double *sp;
  lib1770_pre_t *pre;
  lib1770_block_t *momentary;
  lib1770_block_t *shortterm;
  lib1770_sample_t sample;
#if defined (FFSOX_LFE_CHANNEL) // [
  int lfe;
#endif // ]
};

int ffsox_collect_create(ffsox_collect_t *collect, ffsox_collect_config_t *cc);
void ffsox_collect_cleanup(ffsox_collect_t *collect);

void ffsox_collect_flush(ffsox_collect_t *collect);
void ffsox_collect_channel(void *data, int ch, double x);
void ffsox_collect_sample(void *data);
void ffsox_collect_truepeak(void *data, double x);

///////////////////////////////////////////////////////////////////////////////
struct ffsox_analyze_config {
  const char *path;
  ffsox_aggregate_t *aggregate;
  int stereo;
  double drc;
  ffsox_block_config_t momentary;
  ffsox_block_config_t shortterm;
  FILE *f;
  int dump;
  int ai;
  int vi;
#if defined (_WIN32) // [
  int utf16;
#endif // ]
#if defined (FFSOX_LFE_CHANNEL) // [
  int lfe;
#endif // ]
};

int ffsox_analyze(ffsox_analyze_config_t *ac, int ai, int vi);

/// node //////////////////////////////////////////////////////////////////////
#define FFSOX_STATE_RUN      0
#define FFSOX_STATE_FLUSH    1
#define FFSOX_STATE_END      2

struct ffsox_node_vmt {
  union {
#define FFSOX_NODE_PARENT_VMT
    FFSOX_NODE_PARENT_VMT

    struct {
#define FFSOX_NODE_VMT(T,P) \
      const P *parent; \
      const char *name; \
      void (*cleanup)(T *n); \
      ffsox_node_t *(*prev)(T *n); \
      ffsox_node_t *(*next)(T *n); \
      int (*run)(T *n);
      FFSOX_NODE_VMT(ffsox_node_t,void)
    };
  };
};

struct ffsox_node {
  union {
#define FFSOX_NODE_PARENT_MEM
    FFSOX_NODE_PARENT_MEM

    struct {
#define FFSOX_NODE_MEM(T) \
      const T *vmt; \
      int state;
      FFSOX_NODE_MEM(ffsox_node_vmt_t)
    };
  };
};

int ffsox_node_create(ffsox_node_t *node);
void ffsox_node_destroy(ffsox_node_t *n);
const ffsox_node_vmt_t *ffsox_node_get_vmt(void);

/// source ////////////////////////////////////////////////////////////////////
/*
 * A "source" represents a set of streams. Each stream is represened by
 * a "packet consumer" held in the "consumer" list. The field "next" is
 * updated from the "consumer" list as packets are read.
 *
 * Each entry in the "consumer" list is the head of a double linked list
 * of nodes (states) which are linked by "prev" and "next" fields.
 */
typedef void (*ffsox_source_callback_t)(const ffsox_source_t *, void *);

struct ffsox_source_vmt {
  union {
#define FFSOX_SOURCE_PARENT_VMT \
    FFSOX_NODE_PARENT_VMT \
    ffsox_node_vmt_t node;
    FFSOX_SOURCE_PARENT_VMT

    struct {
#define FFSOX_SOURCE_VMT(T,P) \
      FFSOX_NODE_VMT(T,P)
      FFSOX_SOURCE_VMT(ffsox_source_t,ffsox_node_vmt_t)
    };
  };
};

struct ffsox_source {
  union {
#define FFSOX_SOURCE_PARENT_MEM \
    FFSOX_NODE_PARENT_MEM \
    ffsox_node_t node;
    FFSOX_SOURCE_PARENT_MEM

    struct {
#define FFSOX_SOURCE_MEM(T) \
      FFSOX_NODE_MEM(T) \
      ffsox_format_t f; \
      int ai,vi; \
      ffsox_source_callback_t cb; \
      void *data; \
 \
      struct { \
        ffsox_packet_consumer_list_t *h; \
        ffsox_packet_consumer_list_t *n; \
      } consumer; \
 \
      ffsox_packet_consumer_t *next; \
      int64_t ts; \
      AVPacket pkt;

      FFSOX_SOURCE_MEM(ffsox_source_vmt_t)
    };
  };
};

int ffsox_source_create(ffsox_source_t *n, const char *path, int ai, int vi,
    ffsox_source_callback_t cb, void *data);
const ffsox_source_vmt_t *ffsox_source_get_vmt(void);

int ffsox_source_append(ffsox_source_t *si, ffsox_packet_consumer_t *pc);
int ffsox_source_seek(ffsox_source_t *n, int64_t ts);

int ffsox_source_link_create(ffsox_source_t *si, ffsox_sink_t *so,
    int stereo, double drc, int codec_id, int sample_fmt, double q);
void ffsox_source_link_cleanup(ffsox_source_t *si);
void ffsox_source_progress(const ffsox_source_t *si, /* FILE */void *data);

/// packet_consumer ///////////////////////////////////////////////////////////
struct ffsox_packet_consumer_vmt {
  union {
#define FFSOX_PACKET_CONSUMER_PARENT_VMT \
    FFSOX_NODE_PARENT_VMT \
    ffsox_node_vmt_t node;
    FFSOX_PACKET_CONSUMER_PARENT_VMT

    struct {
#define FFSOX_PACKET_CONSUMER_VMT(T,P) \
      FFSOX_NODE_VMT(T,P) \
      int (*set_packet)(T *n, AVPacket *pkt);
      FFSOX_PACKET_CONSUMER_VMT(ffsox_packet_consumer_t,ffsox_node_vmt_t)
    };
  };
};

struct ffsox_packet_consumer {
  union {
#define FFSOX_PACKET_CONSUMER_PARENT_MEM \
    FFSOX_NODE_PARENT_MEM \
    ffsox_node_t node;
    FFSOX_PACKET_CONSUMER_PARENT_MEM

    struct {
#define FFSOX_PACKET_CONSUMER_MEM(T) \
      FFSOX_NODE_MEM(T) \
      ffsox_stream_t si; \
      ffsox_source_t *prev;
      FFSOX_PACKET_CONSUMER_MEM(ffsox_packet_consumer_vmt_t)
    };
  };
};

#if defined (FFSOX_FIX_883198_MISSING_CODEC_ID) // [
int ffsox_packet_consumer_create(ffsox_packet_consumer_t *n,
    ffsox_source_t *si, int stream_index, int append);
#else // ] [
int ffsox_packet_consumer_create(ffsox_packet_consumer_t *n,
    ffsox_source_t *si, int stream_index);
#endif // ]
const ffsox_packet_consumer_vmt_t *ffsox_packet_consumer_get_vmt(void);

/// packet_writer /////////////////////////////////////////////////////////////
struct ffsox_packet_writer_vmt {
  union {
#define FFSOX_PACKET_WRITER_PARENT_VMT \
    FFSOX_PACKET_CONSUMER_PARENT_VMT \
    ffsox_packet_consumer_vmt_t packet_consumer;
    FFSOX_PACKET_WRITER_PARENT_VMT

    struct {
#define FFSOX_PACKET_WRITER_VMT(T,P) \
      FFSOX_PACKET_CONSUMER_VMT(T,P)
      FFSOX_PACKET_WRITER_VMT(ffsox_packet_writer_t,
          ffsox_packet_consumer_vmt_t)
    };
  };
};

struct ffsox_packet_writer {
  union {
#define FFSOX_PACKET_WRITER_PARENT_MEM \
    FFSOX_PACKET_CONSUMER_PARENT_MEM \
    ffsox_packet_consumer_t packet_consumer;
    FFSOX_PACKET_WRITER_PARENT_MEM

    struct {
#define FFSOX_PACKET_WRITER_MEM(T) \
      FFSOX_PACKET_CONSUMER_MEM(T) \
      ffsox_stream_t so;
      FFSOX_PACKET_WRITER_MEM(ffsox_packet_writer_vmt_t)
    };
  };
};

int ffsox_packet_writer_create(ffsox_packet_writer_t *n, ffsox_source_t *si,
    int stream_index, ffsox_sink_t *so);
ffsox_packet_writer_t *ffsox_packet_writer_new(ffsox_source_t *si,
    int stream_index, ffsox_sink_t *so);
const ffsox_packet_writer_vmt_t *ffsox_packet_writer_get_vmt(void);

/// frame_reader //////////////////////////////////////////////////////////////
struct ffsox_frame_reader_vmt {
  union {
#define FFSOX_FRAME_READER_PARENT_VMT \
    FFSOX_PACKET_CONSUMER_PARENT_VMT \
    ffsox_packet_consumer_vmt_t packet_consumer;
    FFSOX_FRAME_READER_PARENT_VMT

    struct {
#define FFSOX_FRAME_READER_VMT(T,P) \
      FFSOX_PACKET_CONSUMER_VMT(T,P)
      FFSOX_FRAME_READER_VMT(ffsox_frame_reader_t,
          ffsox_packet_consumer_vmt_t)
    };
  };
};

struct ffsox_frame_reader {
  union {
#define FFSOX_FRAME_READER_PARENT_MEM \
    FFSOX_PACKET_CONSUMER_PARENT_MEM \
    ffsox_packet_consumer_t packet_consumer;
    FFSOX_FRAME_READER_PARENT_MEM

    struct {
#define FFSOX_FRAME_READER_MEM(T) \
      FFSOX_PACKET_CONSUMER_MEM(T) \
      AVPacket pkt; \
      ffsox_frame_t fo; \
      ffsox_frame_consumer_t *next;
      FFSOX_FRAME_READER_MEM(ffsox_frame_reader_vmt_t)
    };
  };
};

int ffsox_frame_reader_create(ffsox_frame_reader_t *fr, ffsox_source_t *si,
    int stream_index, int stereo, double drc);
ffsox_frame_reader_t *ffsox_frame_reader_new(ffsox_source_t *si,
    int stream_index, int stereo, double drc);
const ffsox_frame_reader_vmt_t *ffsox_frame_reader_get_vmt(void);

/// frame_consumer ////////////////////////////////////////////////////////////
struct ffsox_frame_consumer_vmt {
  union {
#define FFSOX_FRAME_CONSUMER_PARENT_VMT \
    FFSOX_NODE_PARENT_VMT \
    ffsox_node_vmt_t node;
    FFSOX_FRAME_CONSUMER_PARENT_VMT

    struct {
#define FFSOX_FRAME_CONSUMER_VMT(T,P) \
      FFSOX_NODE_VMT(T,P) \
      int (*set_frame)(T *n, ffsox_frame_t *fi);
      FFSOX_FRAME_CONSUMER_VMT(ffsox_frame_consumer_t,ffsox_node_vmt_t)
    };
  };
};

struct ffsox_frame_consumer {
  union {
#define FFSOX_FRAME_CONSUMER_PARENT_MEM \
    FFSOX_NODE_PARENT_MEM \
    ffsox_node_t node;
    FFSOX_FRAME_CONSUMER_PARENT_MEM

    struct {
#define FFSOX_FRAME_CONSUMER_MEM(T) \
      FFSOX_NODE_MEM(T) \
      ffsox_node_t *prev; \
      ffsox_frame_t *fi;
      FFSOX_FRAME_CONSUMER_MEM(ffsox_frame_consumer_vmt_t)
    };
  };
};

int ffsox_frame_consumer_create(ffsox_frame_consumer_t *n);
const ffsox_frame_consumer_vmt_t *ffsox_frame_consumer_get_vmt(void);

/// frame_writer ////////////////////////////////////////////////////////////
struct ffsox_frame_writer_vmt {
  union {
#define FFSOX_FRAME_WRITER_PARENT_VMT \
    FFSOX_FRAME_CONSUMER_PARENT_VMT \
    ffsox_frame_consumer_vmt_t frame_consumer;
    FFSOX_FRAME_WRITER_PARENT_VMT

    struct {
#define FFSOX_FRAME_WRITER_VMT(T,P) \
      FFSOX_FRAME_CONSUMER_VMT(T,P)
      FFSOX_FRAME_WRITER_VMT(ffsox_frame_writer_t,ffsox_frame_consumer_vmt_t)
    };
  };
};

struct ffsox_frame_writer {
  union {
#define FFSOX_FRAME_WRITER_PARENT_MEM \
    FFSOX_FRAME_CONSUMER_PARENT_MEM \
    ffsox_frame_consumer_t frame_consumer;
    FFSOX_FRAME_WRITER_PARENT_MEM

    struct {
#define FFSOX_FRAME_WRITER_MEM(T) \
      FFSOX_FRAME_CONSUMER_MEM(T) \
      double q; \
      ffsox_stream_t so; \
      ffsox_frame_t fo; \
      AVPacket pkt;
      FFSOX_FRAME_WRITER_MEM(ffsox_frame_writer_vmt_t)
    };
  };
};

int ffsox_frame_writer_create(ffsox_frame_writer_t *fc, ffsox_sink_t *so,
    ffsox_frame_reader_t *fr, int codec_id, int sample_fmt, double q);
ffsox_frame_writer_t *ffsox_frame_writer_new(ffsox_sink_t *so,
    ffsox_frame_reader_t *fr, int codec_id, int sample_fmt, double q);
const ffsox_frame_writer_vmt_t *ffsox_frame_writer_get_vmt(void);

#if defined (_WIN32) // {
/// audio_player ////////////////////////////////////////////////////////////
#define FFSOX_AUDIO_PLAYER_RENDER   1
#define FFSOX_AUDIO_PLAYER_END      2

struct ffsox_audio_player_vmt {
  union {
#define FFSOX_AUDIO_PLAYER_PARENT_VMT \
    FFSOX_FRAME_CONSUMER_PARENT_VMT \
    ffsox_frame_consumer_vmt_t frame_consumer;
    FFSOX_AUDIO_PLAYER_PARENT_VMT

    struct {
#define FFSOX_AUDIO_PLAYER_VMT(T,P) \
      FFSOX_FRAME_CONSUMER_VMT(T,P)
      FFSOX_AUDIO_PLAYER_VMT(ffsox_audio_player_t,ffsox_frame_consumer_vmt_t)
    };
  };
};

struct ffsox_audio_player {
  union {
#define FFSOX_AUDIO_PLAYER_PARENT_MEM \
    FFSOX_FRAME_CONSUMER_PARENT_MEM \
    ffsox_frame_consumer_t frame_consumer;
    FFSOX_AUDIO_PLAYER_PARENT_MEM

    struct {
#define FFSOX_AUDIO_PLAYER_MEM(T) \
      FFSOX_FRAME_CONSUMER_MEM(T) \
      double q; \
 \
      struct { \
        int state; \
        HANDLE hMutex; \
        HANDLE hEvent; \
      } sync; \
 \
      ffsox_frame_t fo; \
      IAudioClient *pAudioClient; \
      WAVEFORMATEXTENSIBLE wfx; \
      REFERENCE_TIME hnsDuration; \
      int nb_samples; \
      IAudioRenderClient *pRenderClient;
      FFSOX_AUDIO_PLAYER_MEM(ffsox_audio_player_vmt_t)
    };
  };
};

int ffsox_audio_player_create(ffsox_audio_player_t *fc,
    ffsox_frame_reader_t *fr, double q, IMMDevice *pDevice,
    AUDCLNT_SHAREMODE eShareMode);
ffsox_audio_player_t *ffsox_audio_player_new(ffsox_frame_reader_t *fr,
    double q, IMMDevice *pDevice, AUDCLNT_SHAREMODE eShareMode);
const ffsox_audio_player_vmt_t *ffsox_audio_player_get_vmt(void);

int ffsox_audio_player_play(ffsox_audio_player_t *ap);
void ffsox_audio_player_kill(ffsox_audio_player_t *ap);
#endif // }

/// sox_reader ////////////////////////////////////////////////////////////
struct ffsox_sox_reader_vmt {
  union {
#define FFSOX_SOX_READER_PARENT_VMT \
    FFSOX_FRAME_CONSUMER_PARENT_VMT \
    ffsox_frame_consumer_vmt_t frame_consumer;
    FFSOX_SOX_READER_PARENT_VMT

    struct {
#define FFSOX_SOX_READER_VMT(T,P) \
      FFSOX_FRAME_CONSUMER_VMT(T,P)
      FFSOX_SOX_READER_VMT(ffsox_sox_reader_t,ffsox_frame_consumer_vmt_t)
    };
  };
};

struct ffsox_sox_reader {
  union {
#define FFSOX_SOX_READER_PARENT_MEM \
    FFSOX_FRAME_CONSUMER_PARENT_MEM \
    ffsox_frame_consumer_t frame_consumer;
    FFSOX_SOX_READER_PARENT_MEM

    struct {
#define FFSOX_SOX_READER_MEM(T) \
      FFSOX_FRAME_CONSUMER_MEM(T) \
      sox_encodinginfo_t encoding; \
      sox_signalinfo_t signal; \
      ffsox_frame_t fo; \
      ffsox_frame_consumer_t *next; \
      double q; \
      ffsox_intercept_t *intercept; \
      sox_uint64_t clips; \
      int sox_errno;
      FFSOX_SOX_READER_MEM(ffsox_sox_reader_vmt_t)
    };
  };
};

int ffsox_sox_reader_create(ffsox_sox_reader_t *sa,
    ffsox_frame_reader_t *fr, double q, ffsox_intercept_t *intercept);
ffsox_sox_reader_t *ffsox_sox_reader_new(ffsox_frame_reader_t *fr,
    double q, ffsox_intercept_t *intercept);
const ffsox_sox_reader_vmt_t *ffsox_sox_reader_get_vmt(void);

size_t ffsox_sox_reader_read(ffsox_sox_reader_t *sa, sox_sample_t *buf,
    size_t len);

#ifdef __cpluplus
}
#endif
#endif // ]
