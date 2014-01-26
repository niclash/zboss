/***************************************************************************
*                      ZBOSS ZigBee Pro 2007 stack                         *
*                                                                          *
*          Copyright (c) 2012 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*          Copyright (c) 2011 ClarIDy Solutions, Inc., Taipei, Taiwan.     *
*                       http://www.claridy.com/                            *
*                                                                          *
*          Copyright (c) 2011 Uniband Electronic Corporation (UBEC),       *
*                             Hsinchu, Taiwan.                             *
*                       http://www.ubec.com.tw/                            *
*                                                                          *
*          Copyright (c) 2011 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*                                                                          *
*                                                                          *
* ZigBee Pro 2007 stack, also known as ZBOSS (R) ZB stack is available     *
* under either the terms of the Commercial License or the GNU General      *
* Public License version 2.0.  As a recipient of ZigBee Pro 2007 stack, you*
* may choose which license to receive this code under (except as noted in  *
* per-module LICENSE files).                                               *
*                                                                          *
* ZBOSS is a registered trademark of DSR Corporation AKA Data Storage      *
* Research LLC.                                                            *
*                                                                          *
* GNU General Public License Usage                                         *
* This file may be used under the terms of the GNU General Public License  *
* version 2.0 as published by the Free Software Foundation and appearing   *
* in the file LICENSE.GPL included in the packaging of this file.  Please  *
* review the following information to ensure the GNU General Public        *
* License version 2.0 requirements will be met:                            *
* http://www.gnu.org/licenses/old-licenses/gpl-2.0.html.                   *
*                                                                          *
* Commercial Usage                                                         *
* Licensees holding valid ClarIDy/UBEC/DSR Commercial licenses may use     *
* this file in accordance with the ClarIDy/UBEC/DSR Commercial License     *
* Agreement provided with the Software or, alternatively, in accordance    *
* with the terms contained in a written agreement between you and          *
* ClarIDy/UBEC/DSR.                                                        *
*                                                                          *
****************************************************************************
PURPOSE: NVRAM functions for c8051f12x
*/
#if !defined ZB_NS_BUILD && defined C8051F120 && defined ZB_USE_NVRAM


#include "zb_common.h"
#include "zb_mac_transport.h"
#include "zb_bufpool.h"
#include "zb_ringbuffer.h"
#include "zb_aps_globals.h"

/* security section is at the end of volatile page */

#include "zb_bank_common.h"

void zb_flash_init()
{
  /* for deubg purpose only 
  RSTSRC = 0x02;
  */
}
/* Erasing nvram 
Step 1. Disable interrupts.
Step 2. If erasing a page in Bank 1, Bank 2, or Bank 3, set the COBANK bits (PSBANK.5-4) for 
the appropriate bank.
Step 3. If erasing a page in the Scratchpad area, set the SFLE bit (PSCTL.2).
Step 4. Set FLWE (FLSCL.0) to enable Flash writes/erases via user software.
Step 5. Set PSEE (PSCTL.1) to enable Flash erases.
Step 6. Set PSWE (PSCTL.0) to redirect MOVX commands to write to Flash.
Step 7. Use the MOVX instruction to write a data byte to any location within the page to be 
erased.
Step 8. Clear PSEE to disable Flash erases.
Step 9. Clear the PSWE bit to redirect MOVX commands to the XRAM data space.
Step 10. Clear the FLWE bit, to disable Flash writes/erases.
Step 11. If erasing a page in the Scratchpad area, clear the SFLE bit.
Step 12. Re-enable interrupts. */

void zb_erase_nvram(zb_uint8_t page)
{
 
    FLSCL |= 0x01;                      /* enable FLASH write/erase */
    PSCTL |= 0x03;                      /* enable erasing FLASH */
    PSCTL |= 0x04;                      /* redirect erasing FLASH to scratchpad FLASH */

    /* writing anywhere initiates erase of the whole page, scratch pad pages are 128 instead of 256 bytes    */    
    ZB_ASSERT(page==0||page==1);
    ZB_XDATA_MEM[129*page] = 0x00;         
    PSCTL &= ~0x07; /* set PSWE = PSEE = SFLE = 0 to disable all access to scratchpad FLASH in place of xdata*/
    FLSCL &= ~0x01; /* disable FLASH write/erase */  
}

/*  writing nvram
Step 1. Disable interrupts.
Step 2. Clear CHBLKW (CCH0CN.0) to select single-byte write mode.
Step 3. If writing to bytes in Bank 1, Bank 2, or Bank 3, set the COBANK bits (PSBANK.5-4) f
the appropriate bank.
Step 4. If writing to bytes in the Scratchpad area, set the SFLE bit (PSCTL.2).
Step 5. Set FLWE (FLSCL.0) to enable Flash writes/erases via user software.
Step 6. Set PSWE (PSCTL.0) to redirect MOVX commands to write to Flash.
Step 7. Use the MOVX instruction to write a data byte to the desired location (repeat as 
necessary).
Step 8. Clear the PSWE bit to redirect MOVX commands to the XRAM data space.
Step 9. Clear the FLWE bit, to disable Flash writes/erases.
Step 10. If writing to bytes in the Scratchpad area, clear the SFLE bit.
Step 11. Re-enable interrupts. */




zb_uint8_t zb_write_nvram(zb_uint8_t pos, zb_uint8_t *buf, zb_uint8_t len )
{    
    zb_uint8_t data i;
    ZB_DISABLE_ALL_INTER();
    FLSCL |= 0x01;                      /* enable FLASH write/erase */
    PSCTL |= 0x01;                      /* enable writing to FLASH in place of xdata */
    PSCTL |= 0x04;                      /* enable writing to scratchpad FLASH instead of to FLASH */

    for (i=0;i<len;i++)
    {
      ZB_XDATA_MEM[pos+i]=buf[i];
    }
    
    PSCTL &= ~0x05;                     /* disable writing to scratchpad or regular FLASH in place of xdata */
    FLSCL &= ~0x01;                     /* disable FLASH write/erase */
    ZB_ENABLE_ALL_INTER();
    return len;
}

zb_uint8_t zb_read_nvram(zb_uint8_t pos, zb_uint8_t *buf, zb_uint8_t len)
{    
    zb_uint8_t data i;  
    ZB_DISABLE_ALL_INTER();
    PSCTL |= 0x04;                      /* enable reading from the scratchpad FLASH instead of from FLASH */
    for (i=0; i<len; ++i)
    {
        buf[i]=ZB_CODE_MEM[pos+i];
    }
    PSCTL &= ~0x04;                     /* disable reading from the scratchpad FLASH instead of from FLASH  */
    ZB_ENABLE_ALL_INTER();
    return len;
}

/* test, if we could write this segment without erasing whole page */
zb_ret_t zb_test_nvram(zb_uint8_t pos, zb_uint8_t *buf, zb_uint8_t len)
{
  zb_uint8_t i = 0;
  zb_uint8_t tmp = 0x00;
  zb_read_nvram(pos, buf, len);
  for (i = 0; i <len ; i++)
  {
    tmp|=~buf[i];
  }
  if (tmp) return RET_ERROR;
  else return RET_OK;
}



zb_ret_t zb_write_nvram_config(zb_uint8_t aps_designated_coord, zb_uint8_t aps_use_insecure_join, zb_uint8_t aps_use_extended_pan_id, 
zb_ieee_addr_t mac_extended_address)
{
  zb_uint8_t written_bytes = 0; 
  zb_uint8_t buf=0x00;
  zb_uint8_t tmp_buf[ZB_SCRATCHPAD_PAGE_SIZE - ZB_CONFIG_SIZE];
  zb_ret_t zb_nvram_save;
  
  zb_nvram_save = zb_test_nvram(ZB_CONFIG_PAGE + ZB_CONFIG_SIZE, tmp_buf, sizeof(tmp_buf));
  zb_erase_nvram(0);
  buf|=(aps_designated_coord|(aps_use_insecure_join<<1)|(aps_use_extended_pan_id<<2));
  written_bytes+=zb_write_nvram(ZB_CONFIG_PAGE, &buf, 1);
  written_bytes+=zb_write_nvram(ZB_CONFIG_PAGE + written_bytes, (zb_uint8_t *) mac_extended_address, sizeof(zb_ieee_addr_t)); 
/* Descriptors moved from nvram, coz there's no reason to store data, which can be hardcoded.
   ***DO NOT FORGET TO MODIFY ZB_CONFIG_SIZE***
  written_bytes+=zb_write_nvram(ZB_CONFIG_PAGE + written_bytes, (zb_uint8_t *) node_desc, sizeof(zb_node_desc_t)); 
  written_bytes+=zb_write_nvram(ZB_CONFIG_PAGE + written_bytes, (zb_uint8_t *) pwr_desc, sizeof(zb_power_desc_t)); 
  written_bytes+=zb_write_nvram(ZB_CONFIG_PAGE + written_bytes, (zb_uint8_t *) simple_desc, sizeof(zb_simple_desc_t)); 
*/ 
  if (zb_nvram_save!=RET_OK) zb_write_nvram(ZB_CONFIG_PAGE + ZB_CONFIG_SIZE, tmp_buf, sizeof(tmp_buf));
  
return RET_OK;
}

zb_ret_t zb_config_from_nvram()
{
  int i;
  zb_uint8_t read_bytes = 0;
  zb_uint8_t buf[8];
  read_bytes+=zb_read_nvram(ZB_CONFIG_PAGE, buf, 1);
  ZB_AIB().aps_designated_coordinator = buf[0]&0x01;
  ZB_AIB().aps_insecure_join = buf[0]&0x02;
  /* TODO: insert ctx update here */
  // ZB_AIB().aps_use_extended_pan_id = buf[0]&0x04;
  //
  read_bytes+=zb_read_nvram(ZB_CONFIG_PAGE+read_bytes, buf, sizeof(zb_ieee_addr_t));
  for (i=0;i<sizeof(zb_ieee_addr_t);i++)
  {
    MAC_PIB().mac_extended_address[i]= buf[i];  
  }
  
  ZB_UPDATE_LONGMAC();

  /* TODO: insert ctx update here */
  /* removed from nvram
  read_bytes+=zb_read_nvram(ZB_CONFIG_PAGE+read_bytes, buf, sizeof(zb_node_desc_t));

  read_bytes+=zb_read_nvram(ZB_CONFIG_PAGE+read_bytes, buf, sizeof(zb_power_desc_t));

  read_bytes+=zb_read_nvram(ZB_CONFIG_PAGE+read_bytes, buf, sizeof(zb_simple_desc_t));
  */
  return RET_OK;
}


zb_ret_t zb_write_formdesc_data(zb_uint8_t profile_in_use, zb_ieee_addr_t long_parent_addr, zb_uint32_t aps_channel_mask,
zb_uint16_t short_parent_addr, zb_uint8_t     depth, zb_uint16_t pan_id, zb_ext_pan_id_t ext_pan_id, zb_uint16_t nwk_short_addr)
{                                                     
  zb_uint8_t written_bytes = 0; 
  zb_uint8_t buf=0x00;
  zb_uint8_t zb_nvram_save = 0;

  zb_uint8_t tmp_buf_config[ZB_CONFIG_SIZE];
  zb_uint8_t tmp_buf_keys[ZB_CCM_KEY_SIZE*(ZB_SECUR_N_SECUR_MATERIAL)];
  zb_read_nvram(ZB_CONFIG_PAGE, tmp_buf_config, ZB_CONFIG_SIZE);
  zb_nvram_save = zb_read_nvram(ZB_CONFIG_PAGE+ZB_SCRATCHPAD_PAGE_SIZE - sizeof(tmp_buf_keys), tmp_buf_keys, sizeof(tmp_buf_keys));  
  
  written_bytes+=ZB_CONFIG_SIZE; /* skip config section */
  written_bytes+=zb_write_nvram(ZB_CONFIG_PAGE + written_bytes, (zb_uint8_t *)&profile_in_use, sizeof(zb_uint8_t));
  written_bytes+=zb_write_nvram(ZB_CONFIG_PAGE + written_bytes, (zb_uint8_t *)long_parent_addr, sizeof(zb_ieee_addr_t));  
  written_bytes+=zb_write_nvram(ZB_CONFIG_PAGE + written_bytes, (zb_uint8_t *)&aps_channel_mask, sizeof(zb_uint32_t));
  written_bytes+=zb_write_nvram(ZB_CONFIG_PAGE + written_bytes, (zb_uint8_t *)&short_parent_addr, sizeof(zb_uint16_t));
  written_bytes+=zb_write_nvram(ZB_CONFIG_PAGE + written_bytes, (zb_uint8_t *)&depth, sizeof(zb_uint8_t));
  written_bytes+=zb_write_nvram(ZB_CONFIG_PAGE + written_bytes, (zb_uint8_t *)&pan_id, sizeof(zb_uint16_t));
  written_bytes+=zb_write_nvram(ZB_CONFIG_PAGE + written_bytes, (zb_uint8_t *)ext_pan_id, sizeof(zb_ext_pan_id_t));
  written_bytes+=zb_write_nvram(ZB_CONFIG_PAGE + written_bytes, (zb_uint8_t *)&nwk_short_addr, sizeof(zb_uint16_t));
  /* restore security & config section */
  zb_write_nvram(ZB_CONFIG_PAGE, tmp_buf_config, ZB_CONFIG_SIZE);
  if (zb_nvram_save)
  {
    zb_write_nvram(ZB_CONFIG_PAGE+ZB_SCRATCHPAD_PAGE_SIZE - sizeof(tmp_buf_keys), tmp_buf_keys, sizeof(tmp_buf_keys));  
  }
  return RET_OK;
}

#pragma OT(0)
zb_ret_t zb_read_formdesc_data()
{
  zb_uint8_t read_bytes = 0;
  zb_uint8_t buf[8];
  zb_ieee_addr_t parent_addr;
  zb_uint16_t s_parent_addr;
  read_bytes = ZB_CONFIG_SIZE;

  /* Stack profile */
  read_bytes+=zb_read_nvram(ZB_CONFIG_PAGE + read_bytes, buf, sizeof(zb_uint8_t));    
  /* Parent's long addr */
  read_bytes+=zb_read_nvram(ZB_CONFIG_PAGE + read_bytes, buf, sizeof(zb_ieee_addr_t));  
  /* this can be replaced by memcmp, but it will cost more code space*/
  if ((buf[0]&buf[1]&buf[2]&buf[3]&buf[4]&buf[5]&buf[6]&buf[7])!=0xFF)
  {
	  zb_neighbor_tbl_ent_t *nbt;
	  ZB_IEEE_ADDR_COPY(parent_addr, buf);
	  /* channel mask */
  read_bytes+=zb_read_nvram(ZB_CONFIG_PAGE + read_bytes, buf, sizeof(zb_uint32_t));
	  ZB_AIB().aps_channel_mask = *(zb_uint32_t*)buf;
	  /* Parent's short address */
  read_bytes+=zb_read_nvram(ZB_CONFIG_PAGE + read_bytes, buf, sizeof(zb_uint16_t));
	  s_parent_addr = *(zb_uint16_t*)buf;
	  /*we have long and short parent's address, so we can update neighbour table now */
	  zb_address_update(parent_addr, s_parent_addr,ZB_FALSE, &ZG->nwk.handle.parent);
	  zb_nwk_neighbor_get(ZG->nwk.handle.parent, ZB_TRUE, &nbt);
	  nbt->relationship = ZB_NWK_RELATIONSHIP_PARENT;
	  nbt->device_type = ZB_NWK_DEVICE_TYPE_COORDINATOR;
	  nbt->rx_on_when_idle = ZB_TRUE;
	  nbt->addr_ref = ZG->nwk.handle.parent;
      

	  /* Device depth */
  read_bytes+=zb_read_nvram(ZB_CONFIG_PAGE + read_bytes, buf, sizeof(zb_uint8_t));
	  ZB_NIB_DEPTH() = buf[0];
	  /* Short PanID */
  read_bytes+=zb_read_nvram(ZB_CONFIG_PAGE + read_bytes, buf, sizeof(zb_uint16_t));
	  MAC_PIB().mac_pan_id = *(zb_uint16_t*)buf;
	  ZB_UPDATE_PAN_ID();
	  /* Extended PanID */
	  read_bytes+=zb_read_nvram(ZB_CONFIG_PAGE + read_bytes, buf, sizeof(zb_ext_pan_id_t));	  
	  ZB_IEEE_ADDR_COPY(ZG->nwk.nib.extended_pan_id, buf);
	  /* Short address */ 
  read_bytes+=zb_read_nvram(ZB_CONFIG_PAGE + read_bytes, buf, sizeof(zb_uint16_t));
	  MAC_PIB().mac_short_address = *(zb_uint16_t *)buf;
	  ZB_UPDATE_SHORT_ADDR();
/* define to pass bv-31, it contradicts specification, so this hack only for test*/
#ifdef TP_PRO_BV_31
	{
	  zb_uint8_t channel_number;
	  /*We're joined already */	  
	  ZG->nwk.handle.joined = 1;
	  for(channel_number = ZB_MAC_START_CHANNEL_NUMBER; channel_number <= ZB_MAC_MAX_CHANNEL_NUMBER; channel_number++)
	  {
	      /* check channel mask */
	      if (ZB_DEFAULT_APS_CHANNEL_MASK & (1l << channel_number))
	      {	   
	        ZB_TRANSCEIVER_SET_CHANNEL(channel_number);
			break;
	      }
	   }	 
	   ZB_AIB_APS_COUNTER() = 0xF0;
	}
#endif 
  }
  return RET_OK;
}



#ifdef ZB_SECURITY
zb_ret_t zb_write_security_key()
{
  /* security keys and counter stored at the end of volatile data page */
  int i;
  zb_uint8_t tmp_buf[ZB_SCRATCHPAD_PAGE_SIZE - ZB_CCM_KEY_SIZE * (ZB_SECUR_N_SECUR_MATERIAL)];
  zb_uint8_t zb_nvram_save = 0;
  zb_read_nvram(ZB_CONFIG_PAGE, tmp_buf, sizeof(tmp_buf));
  zb_erase_nvram(ZB_CONFIG_PAGE);
  for (i = 0; i<ZB_SECUR_N_SECUR_MATERIAL;i++)
  {
    zb_write_nvram(ZB_CONFIG_PAGE+ZB_SCRATCHPAD_PAGE_SIZE-ZB_CCM_KEY_SIZE*((ZB_SECUR_N_SECUR_MATERIAL)-i), 
    ZG->nwk.nib.secur_material_set[i].key, ZB_CCM_KEY_SIZE);
  }
  zb_write_nvram(ZB_CONFIG_PAGE, tmp_buf, sizeof(tmp_buf));
  return RET_OK;
}

zb_ret_t zb_read_security_key()
{  
  int i;
  for (i = 0; i<ZB_SECUR_N_SECUR_MATERIAL;i++)
  {
    zb_read_nvram(ZB_CONFIG_PAGE+ZB_SCRATCHPAD_PAGE_SIZE - ZB_CCM_KEY_SIZE*((ZB_SECUR_N_SECUR_MATERIAL)-i), 
    ZG->nwk.nib.secur_material_set[i].key, ZB_CCM_KEY_SIZE);
  }
  return RET_OK;
}
zb_ret_t zb_write_up_counter()
{
   zb_uint8_t buf[ZB_SCRATCHPAD_PAGE_SIZE];
   zb_uint8_t i;

   zb_read_nvram(ZB_VOLATILE_PAGE, buf, ZB_SCRATCHPAD_PAGE_SIZE);   
   if (buf[ZB_SCRATCHPAD_PAGE_SIZE-3]!=0xFF) 
      {
             zb_erase_nvram(1);
             zb_write_nvram(ZB_VOLATILE_PAGE, (zb_uint8_t *) &ZG->nwk.nib.outgoing_frame_counter, sizeof(ZG->nwk.nib.outgoing_frame_counter));
      }   
   else
   {
    for (i = ZB_SCRATCHPAD_PAGE_SIZE; i>=0; i--)
    {
        if (((buf[i]!=0xFF)&&(i<ZB_SCRATCHPAD_PAGE_SIZE))||(i == 0))
        {
          zb_write_nvram(ZB_VOLATILE_PAGE+i+1*(i&0x01), (zb_uint8_t *) &ZG->nwk.nib.outgoing_frame_counter, sizeof(ZG->nwk.nib.outgoing_frame_counter));
          break;
        }
    }
   }
   return RET_OK;  
}

zb_ret_t zb_read_up_counter()
{
   zb_uint8_t i;
   zb_uint8_t buf[ZB_SCRATCHPAD_PAGE_SIZE]; 
   zb_read_nvram(ZB_VOLATILE_PAGE, buf, ZB_SCRATCHPAD_PAGE_SIZE);   
   for (i = ZB_SCRATCHPAD_PAGE_SIZE-1; i>=0; i--)
   {
      if ((buf[i]!=0xFF)&&(i>3)) /* i>3 just check if some garbage in nvram, because we always put an 4bytes value*/
      {
        ZG->nwk.nib.outgoing_frame_counter = *(zb_uint32_t*) (buf+i-3);
        break;        
      }
   }
   return RET_OK;  
}
#endif

#endif

