/* devLonginYewPlc.c */
/****************************************************************************
 *                         COPYRIGHT NOTIFICATION
 *
 * devLonginYewPlc: A part of EPICS device support for FA-M3 PLC made by
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

#include        <longinRecord.h>

#ifndef EPICS_REVISION
#include <epicsVersion.h>
#endif
#if EPICS_REVISION == 14 && EPICS_MODIFICATION >= 2
#include <epicsExport.h>
#endif

/***************************************************************
 * Long input (command/response IO)
 ***************************************************************/
LOCAL long init_longin_record(struct longinRecord *);
LOCAL long read_longin(struct longinRecord *);
LOCAL long config_longin_command(struct dbCommon *, int *, uint8_t *, int *, void *, int);
LOCAL long parse_longin_response(struct dbCommon *, int *, uint8_t *, int *, void *, int);

INTEGERDSET devLiYewPlc = {
  5,
  NULL,
  netDevInit,
  init_longin_record,
  NULL,
  read_longin
};

#if EPICS_REVISION == 14 && EPICS_MODIFICATION >= 2
epicsExportAddress(dset, devLiYewPlc);
#endif


LOCAL long init_longin_record(struct longinRecord *plongin)
{
  return netDevInitXxRecord(
			    (struct dbCommon *) plongin,
			    &plongin->inp,
			    MPF_READ | YEW_GET_PROTO | DEFAULT_TIMEOUT,
			    yew_calloc(0, 0, 0, 2),
			    yew_parse_link,
			    config_longin_command,
			    parse_longin_response
			    );
}


LOCAL long read_longin(struct longinRecord *plongin)
{
  return netDevReadWriteXx((struct dbCommon *) plongin);
}


LOCAL long config_longin_command(
				 struct dbCommon *pxx,
				 int *option,
				 uint8_t *buf,
				 int *len,
				 void *device,
				 int transaction_id
				 )
{
  struct longinRecord *plongin = (struct longinRecord *)pxx;
  YEW_PLC *d = (YEW_PLC *) device;

  return yew_config_command(
			    buf,
			    len,
			    &plongin->val, /* not referenced */
			    DBF_LONG,      /* not referenced */
			    (d->dword)? 2:1,
			    option,
			    d
			    );
} 


LOCAL long parse_longin_response(
				 struct dbCommon *pxx,
				 int *option,
				 uint8_t *buf,
				 int *len,
				 void *device,
				 int transaction_id
				 )
{
  struct longinRecord *plongin = (struct longinRecord *)pxx;
  YEW_PLC *d = (YEW_PLC *) device;
  long ret;

  if (d->dword)
    {
      uint16_t u16_val[2];

      ret = yew_parse_response(
			       buf,
			       len,
			       &u16_val[0],
			       DBF_USHORT,
			       2,
			       option,
			       d
			       );

      plongin->val = u16_val[1] << 16 | u16_val[0];
    }
  else
    {
      ret = yew_parse_response(
			       buf,
			       len,
			       &plongin->val,
			       DBF_LONG,
			       1,
			       option,
			       d
			       );
    }

  return ret;
}


