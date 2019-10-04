/*
 * Copyright 2013-2019 Software Radio Systems Limited
 *
 * This file is part of srsLTE.
 *
 * srsLTE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsLTE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#ifndef SRSLTE_PDCP_NR_TEST_H
#define SRSLTE_PDCP_NR_TEST_H

#include "srslte/common/buffer_pool.h"
#include "srslte/common/log_filter.h"
#include "srslte/common/security.h"
#include "srslte/upper/pdcp_entity_nr.h"

#define TESTASSERT(cond)                                                                                               \
  {                                                                                                                    \
    if (!(cond)) {                                                                                                     \
      std::cout << "[" << __FUNCTION__ << "][Line " << __LINE__ << "]: FAIL at " << (#cond) << std::endl;              \
      return -1;                                                                                                       \
    }                                                                                                                  \
  }

struct pdcp_security_cfg {
  uint8_t *k_int_rrc;
  uint8_t *k_enc_rrc;
  uint8_t *k_int_up;
  uint8_t *k_enc_up;
  srslte::INTEGRITY_ALGORITHM_ID_ENUM int_algo;
  srslte::CIPHERING_ALGORITHM_ID_ENUM enc_algo;
};

// dummy classes
class rlc_dummy : public srsue::rlc_interface_pdcp
{
public:
  rlc_dummy(srslte::log* log_) : log(log_) {}

  void get_last_sdu(const srslte::unique_byte_buffer_t& pdu)
  {
    memcpy(pdu->msg, last_pdcp_pdu->msg, last_pdcp_pdu->N_bytes);
    pdu->N_bytes = last_pdcp_pdu->N_bytes;
    return;
  }
  void write_sdu(uint32_t lcid, srslte::unique_byte_buffer_t sdu, bool blocking = true)
  {
    log->info_hex(sdu->msg, sdu->N_bytes, "RLC SDU");
    last_pdcp_pdu.swap(sdu);
  }

private:
  srslte::log*                 log;
  srslte::unique_byte_buffer_t last_pdcp_pdu;

  bool rb_is_um(uint32_t lcid) { return false; }
};

class rrc_dummy : public srsue::rrc_interface_pdcp
{
public:
  rrc_dummy(srslte::log* log_) : log(log_) {}

  void write_pdu(uint32_t lcid, srslte::unique_byte_buffer_t pdu) {}
  void write_pdu_bcch_bch(srslte::unique_byte_buffer_t pdu) {}
  void write_pdu_bcch_dlsch(srslte::unique_byte_buffer_t pdu) {}
  void write_pdu_pcch(srslte::unique_byte_buffer_t pdu) {}
  void write_pdu_mch(uint32_t lcid, srslte::unique_byte_buffer_t pdu) {}

  std::string get_rb_name(uint32_t lcid) { return "None"; }

private:
  srslte::log* log;
};

class gw_dummy : public srsue::gw_interface_pdcp
{
public:
  gw_dummy(srslte::log* log_) : log(log_) {}

  void     write_pdu_mch(uint32_t lcid, srslte::unique_byte_buffer_t pdu) {}
  uint32_t rx_count = 0;

  void get_last_pdu(const srslte::unique_byte_buffer_t& pdu)
  {
    memcpy(pdu->msg, last_pdu->msg, last_pdu->N_bytes);
    pdu->N_bytes = last_pdu->N_bytes;
    return;
  }
  void write_pdu(uint32_t lcid, srslte::unique_byte_buffer_t pdu)
  {
    log->info_hex(pdu->msg, pdu->N_bytes, "GW PDU");
    rx_count++;
    last_pdu.swap(pdu);
  }

private:
  srslte::log*                 log;
  srslte::unique_byte_buffer_t last_pdu;
};

/*
 * Helper classes to reduce copy / pasting in setting up tests
 */
class pdcp_nr_test_helper
{
public:
  pdcp_nr_test_helper(srslte::pdcp_config_t cfg, pdcp_security_cfg sec_cfg, srslte::log* log) :
    rlc(log),
    rrc(log),
    gw(log),
    timers(64)
  {
    pdcp.init(&rlc, &rrc, &gw, &timers, log, 0, cfg);
    pdcp.config_security(
        sec_cfg.k_enc_rrc, sec_cfg.k_int_rrc, sec_cfg.k_enc_up, sec_cfg.k_int_up, sec_cfg.enc_algo, sec_cfg.int_algo);
    pdcp.enable_integrity();
    pdcp.enable_encryption();
  }

  srslte::pdcp_entity_nr pdcp;
  rlc_dummy              rlc;
  rrc_dummy              rrc;
  gw_dummy               gw;
  srslte::timers         timers;
};
#endif // SRSLTE_PDCP_NR_TEST_H