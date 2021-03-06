/*
 * ffsox_packet_consumer.c
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
#include <ffsox_priv.h>

static packet_consumer_vmt_t vmt;

#if defined (FFSOX_FIX_883198_MISSING_CODEC_ID) // [
int ffsox_packet_consumer_create(packet_consumer_t *pc, source_t *si,
    int stream_index, int append)
#else // ] [
int ffsox_packet_consumer_create(packet_consumer_t *pc, source_t *si,
    int stream_index)
#endif // ]
{
  if (ffsox_node_create(&pc->node)<0) {
    DMESSAGE("creating node");
    goto base;
  }

  pc->vmt=ffsox_packet_consumer_get_vmt();
  pc->si.fc=si->f.fc;
  pc->si.stream_index=stream_index;
  pc->si.st=pc->si.fc->streams[stream_index];
  pc->si.cc=pc->si.st->codec;
  pc->si.codec=pc->si.cc->codec;

#if defined (FFSOX_FIX_883198_MISSING_CODEC_ID) // [
  if (append) {
#endif // ]
    // link us to the packet consumer list.
    if (ffsox_source_append(si,pc)<0) {
      DMESSAGE("appending packet consumer");
      goto append;
    }
#if defined (FFSOX_FIX_883198_MISSING_CODEC_ID) // [
  }
#endif // ]

  return 0;
append:
  vmt.parent->cleanup(&pc->node);
base:
  return -1;
}

////////
static node_t *packet_consumer_prev(packet_consumer_t *pc)
{
  return NULL==pc->prev?NULL:&pc->prev->node;
}

static int packet_consumer_set_packet(packet_consumer_t *pc, AVPacket *pkt)
{
  DMESSAGE("not implemented");
  (void)pc;
  (void)pkt;

  return -1;
}

const packet_consumer_vmt_t *ffsox_packet_consumer_get_vmt(void)
{
  const node_vmt_t *parent;

  if (NULL==vmt.parent) {
    parent=ffsox_node_get_vmt();
    vmt.node=*parent;
    vmt.parent=parent;
    vmt.name="packet_consumer";
    vmt.prev=packet_consumer_prev;
    vmt.set_packet=packet_consumer_set_packet;
  }

  return &vmt;
}
