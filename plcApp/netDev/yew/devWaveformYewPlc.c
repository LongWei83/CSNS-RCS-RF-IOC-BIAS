/* devWaveformYewPlc.c */
/****************************************************************************
 *                         COPYRIGHT NOTIFICATION
 *
 * devWaveformYewPlc: A part of EPICS device support for FA-M3 PLC made by
 * Yokogawa
 * Copyright (C) 2004  Jun-ichi Odagiri (KEK)
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
 ****************************************************************************/
/* Author: Jun-ichi Odagiri (jun-ichi.odagiri@kek.jp, KEK) */
/* Modification Log:
 * -----------------
 * 2005/08/22 jio
 *  passing option flag to Link Field Parser was changed from by value to
 * by pointer  
 */

#include        <waveformRecord.h>

#ifndef EPICS_REVISION
#include <epicsVersion.h>
#endif
#if EPICS_REVISION == 14 && EPICS_MODIFICATION >= 2
#include <epicsExport.h>
#endif

/***************************************************************
 * Waveform (command/response IO)
 ***************************************************************/
LOCAL long init_waveform_record(struct waveformRecord *);
LOCAL long read_waveform(struct waveformRecord *);
LOCAL long config_waveform_command(struct dbCommon *, int *, uint8_t *, int *, void *, int);
LOCAL long parse_waveform_response(struct dbCommon *, int *, uint8_t *, int *, void *, int);

INTEGERDSET devWfYewPlc = {
  5,
  NULL,
  netDevInit,
  init_waveform_record,
  NULL,
  read_waveform
};

#if EPICS_REVISION == 14 && EPICS_MODIFICATION >= 2
epicsExportAddress(dset, devWfYewPlc);
#endif

static uint16_t *u16_val;

LOCAL long init_waveform_record(struct waveformRecord *pwf)
{
  u16_val = (uint16_t *) calloc(2*pwf->nelm, sizeofTypes[DBF_USHORT]);

  if (!u16_val)
    {
      errlogPrintf("devWaveformYewPlc: calloc failed\n");
      return ERROR;
    }

  return netDevInitXxRecord(
			    (struct dbCommon *) pwf,
			    &pwf->inp,
			    MPF_READ | YEW_GET_PROTO | DEFAULT_TIMEOUT,
			    yew_calloc(0, 0, 0, 2),
			    yew_parse_link,
			    config_waveform_command,
			    parse_waveform_response
			    );
}


LOCAL long read_waveform(struct waveformRecord *pwf)
{
  TRANSACTION *t = (TRANSACTION *) pwf->dpvt;
  YEW_PLC *d = (YEW_PLC *) t->device;

  /*
   * make sure that those below are cleared in the event that
   * a multi-step transfer is terminated by an error in the
   * middle of transacton
   */
  d->nleft = 0;
  d->noff = 0;

  return netDevReadWriteXx((struct dbCommon *) pwf);
}


LOCAL long config_waveform_command(
				   struct dbCommon *pxx,
				   int *option,
				   uint8_t *buf,
				   int *len,
				   void *device,
				   int transaction_id
				   )
{
  struct waveformRecord *pwf = (struct waveformRecord *)pxx;

  return yew_config_command(
			    buf,
			    len,
			    pwf->bptr,
			    pwf->ftvl,
			    pwf->nelm,
			    option,
			    (YEW_PLC *) device
			    );
} 


LOCAL long parse_waveform_response(
				   struct dbCommon *pxx,
				   int *option,
				   uint8_t *buf,
				   int *len,
				   void *device,
				   int transaction_id
				   )
{
  struct waveformRecord *pwf = (struct waveformRecord *)pxx;
  YEW_PLC *d = (YEW_PLC *) device;
  long ret;

  ret = yew_parse_response(
			   buf,
			   len,
			   pwf->bptr,
			   pwf->ftvl,
			   pwf->nelm,
			   option,
			   d
			   );

  switch (ret)
    {
    case NOT_DONE:
      pwf->nord = d->noff;
    case 0:
      pwf->nord = pwf->nelm;
    default:
      ;
    }

  return ret;
}


