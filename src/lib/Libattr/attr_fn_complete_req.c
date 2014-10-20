/*
*         OpenPBS (Portable Batch System) v2.3 Software License
*
* Copyright (c) 1999-2000 Veridian Information Solutions, Inc.
* All rights reserved.
*
* ---------------------------------------------------------------------------
* For a license to use or redistribute the OpenPBS software under conditions
* other than those described below, or to purchase support for this software,
* please contact Veridian Systems, PBS Products Department ("Licensor") at:
*
*    www.OpenPBS.org  +1 650 967-4675                  sales@OpenPBS.org
*                        877 902-4PBS (US toll-free)
* ---------------------------------------------------------------------------
*
* This license covers use of the OpenPBS v2.3 software (the "Software") at
* your site or location, and, for certain users, redistribution of the
* Software to other sites and locations.  Use and redistribution of
* OpenPBS v2.3 in source and binary forms, with or without modification,
* are permitted provided that all of the following conditions are met.
* After December 31, 2001, only conditions 3-6 must be met:
*
* 1. Commercial and/or non-commercial use of the Software is permitted
*    provided a current software registration is on file at www.OpenPBS.org.
*    If use of this software contributes to a publication, product, or
*    service, proper attribution must be given; see www.OpenPBS.org/credit.html
*
* 2. Redistribution in any form is only permitted for non-commercial,
*    non-profit purposes.  There can be no charge for the Software or any
*    software incorporating the Software.  Further, there can be no
*    expectation of revenue generated as a consequence of redistributing
*    the Software.
*
* 3. Any Redistribution of source code must retain the above copyright notice
*    and the acknowledgment contained in paragraph 6, this list of conditions
*    and the disclaimer contained in paragraph 7.
*
* 4. Any Redistribution in binary form must reproduce the above copyright
*    notice and the acknowledgment contained in paragraph 6, this list of
*    conditions and the disclaimer contained in paragraph 7 in the
*    documentation and/or other materials provided with the distribution.
*
* 5. Redistributions in any form must be accompanied by information on how to
*    obtain complete source code for the OpenPBS software and any
*    modifications and/or additions to the OpenPBS software.  The source code
*    must either be included in the distribution or be available for no more
*    than the cost of distribution plus a nominal fee, and all modifications
*    and additions to the Software must be freely redistributable by any party
*    (including Licensor) without restriction.
*
* 6. All advertising materials mentioning features or use of the Software must
*    display the following acknowledgment:
*
*     "This product includes software developed by NASA Ames Research Center,
*     Lawrence Livermore National Laboratory, and Veridian Information
*     Solutions, Inc.
*     Visit www.OpenPBS.org for OpenPBS software support,
*     products, and information."
*
* 7. DISCLAIMER OF WARRANTY
*
* THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT
* ARE EXPRESSLY DISCLAIMED.
*
* IN NO EVENT SHALL VERIDIAN CORPORATION, ITS AFFILIATED COMPANIES, OR THE
* U.S. GOVERNMENT OR ANY OF ITS AGENCIES BE LIABLE FOR ANY DIRECT OR INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* This license will be governed by the laws of the Commonwealth of Virginia,
* without reference to its choice of law rules.
*/



#include <pbs_config.h>   /* the master config generated by configure */
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>

#include "pbs_error.h"
#include "complete_req.hpp"
#include "attribute.h"
#include "pbs_ifl.h"



void free_complete_req(

  pbs_attribute *patr)

  {
  if (patr->at_val.at_ptr != NULL)
    {
    complete_req *delete_me = (complete_req *)patr->at_val.at_ptr;
    delete delete_me;
    patr->at_val.at_ptr = NULL;
    }
  
  patr->at_flags &= ~ATR_VFLAG_SET;
  } // END free_complete_req()


/*
 * decode_complete_req()
 *
 * saves the complete_req information specified by val in the attribute patr
 *
 * @param patr  - the attribute that should have its req_info set
 * @param name  - optional, not used here
 * @param rescn - not used here
 * @param val   - the string that was output by a call to complete_req::toString()
 * @param perm  - permissions, ignored
 *
 * @return PBSE_NONE if the attribute was successfully decoded, or a pbs error code
 */

int  decode_complete_req(
    
  pbs_attribute *patr,
  const char    *name,
  const char    *rescn,
  const char    *val,
  int            perm)

  {
  if (val == NULL)
    return(PBSE_BAD_PARAMETER);

  if (rescn == NULL)
    {
    complete_req *cr = new complete_req();
    cr->set_from_string(val);

    if (cr->req_count() == 0)
      {
      delete cr;
      return(PBSE_BAD_PARAMETER);
      }

    free_complete_req(patr);

    patr->at_val.at_ptr = cr;
    patr->at_flags |= ATR_VFLAG_SET;

    return(PBSE_NONE);
    }
  else
    {
    complete_req *cr;

    if ((patr->at_val.at_ptr != NULL) &&
        ((patr->at_flags & ATR_VFLAG_SET) != 0))
      cr = (complete_req *)patr->at_val.at_ptr;
    else
      {
      cr = new complete_req();
      patr->at_val.at_ptr = cr;
      patr->at_flags |= ATR_VFLAG_SET;
      }

    char *attr_name = strdup(rescn);
    char *colon = strchr(attr_name, ':');
    int   rc = PBSE_BAD_PARAMETER;
    
    if (colon != NULL)
      {
      int index = strtol(colon + 1, NULL, 10);
      *colon = '\0';
      rc = cr->set_value(index, attr_name, val);
      }

    free(attr_name);

    if (rc != PBSE_NONE)
      {
      free_complete_req(patr);
      return(PBSE_BAD_PARAMETER);
      }

    return(rc);
    }
  } // END decode_complete_req()



/*
 * encode_complete_req()
 *
 * encodes attr's complete_req info into the list that is phead
 *
 * @param attr   - the attribute which holds a complete_req
 * @param phead  - the list where we should encode attr
 * @param atname - the name of the attribute
 * @param rsname - the name of the resource (unused here)
 * @param mode   - the mode (unused here)
 * @param perm   - permissions (unused here)
 * @return PBSE_NONE on success or a pbs error code
 */

int encode_complete_req(
    
  pbs_attribute *attr,
  tlist_head    *phead,
  const char    *atname,
  const char    *rsname,
  int            mode,
  int            perm)

  {
  // if the attribute is empty then nothing should be done
  if ((attr == NULL) ||
      (attr->at_val.at_ptr == NULL))
    return(PBSE_NONE);

  std::vector<std::string>  names;
  std::vector<std::string>  values;
  complete_req             *cr;
  pbs_attribute             tmp;
  int                       rc = PBSE_NONE;

  cr = (complete_req *)attr->at_val.at_ptr;

  cr->get_values(names, values);

  memset(&tmp, 0, sizeof(tmp));
  tmp.at_flags = ATR_VFLAG_SET;

  for (unsigned int i = 0; i < names.size(); i++)
    {
    tmp.at_val.at_str = (char *)values[i].c_str();
    if ((rc = encode_str(&tmp, phead, atname, names[i].c_str(), mode, perm)) < 0)
      break;
    }

  return(rc);
  } // END decode_complete_req()



/*
 * overwrite_complete_req()
 *
 * overwrites the complete_req in attr with the one in new_attr
 *
 * NOTE: parameters have been checked in the parent function set_complete_req
 *
 * @param attr - the attribute being overwritten
 * @param new_attr - the attribute doing the overwriting
 * 
 */

void overwrite_complete_req(

  pbs_attribute *attr,
  pbs_attribute *new_attr)

  {
  complete_req *to_copy = (complete_req *)new_attr->at_val.at_ptr;

  if (attr->at_val.at_ptr == NULL)
    {
    attr->at_val.at_ptr = new complete_req(*to_copy);
    }
  else
    {
    complete_req *cr = (complete_req *)attr->at_val.at_ptr;
    *cr = *to_copy;
    attr->at_val.at_ptr = cr;
    }

  attr->at_flags |= ATR_VFLAG_SET;
  } // END overwrite_complete_req



/*
 * set_complete_req()
 *
 * sets the attribute attr with the value from new_attr
 * @param attr - the attribute being set
 * @param new_attr - the attribute whose value is set in attr
 * @param op - the operation being performed. Right now, only SET and INCR work. INCR only
 * works if attr has no value
 *
 * @return PBSE_NONE on success, or PBSE_INTERNAL on unsupported operations.
 */

int set_complete_req(
    
  pbs_attribute *attr,
  pbs_attribute *new_attr,
  enum batch_op  op)

  {
  if ((attr == NULL) ||
      (new_attr == NULL) ||
      (new_attr->at_val.at_ptr == NULL))
    return(PBSE_BAD_PARAMETER);

  if ((attr->at_val.at_ptr == NULL) &&
      (op == INCR))
    op = SET;

  switch (op)
    {
    case SET:

      overwrite_complete_req(attr, new_attr);

      break;

    case UNSET:

      free_complete_req(attr);

      break;

    default:

      // unsupported
      return(PBSE_INTERNAL);
    }

  return(PBSE_NONE);
  } // END set_complete_req()



int comp_complete_req(
   
  pbs_attribute *attr,
  pbs_attribute *with)

  {
  return(0);
  } // END comp_complete_req()

