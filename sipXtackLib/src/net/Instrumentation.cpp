#include "net/Instrumentation.h"



static const char* system_tap_sip_rx_src;
static unsigned short system_tap_sip_rx_srcPort;
static const char* system_tap_sip_rx_dst;
static unsigned short system_tap_sip_rx_dstPort;
static const char* system_tap_sip_rx_sipMessage;
static int system_tap_sip_rx_size;

void system_tap_sip_rx(
  const char* src,
  unsigned short srcPort,
  const char* dst,
  unsigned short dstPort,
  const char* sipMessage,
  int size)
{
  system_tap_sip_rx_src = src;
  system_tap_sip_rx_srcPort = srcPort;
  system_tap_sip_rx_dst = dst;
  system_tap_sip_rx_dstPort = dstPort;
  system_tap_sip_rx_sipMessage = sipMessage;
  system_tap_sip_rx_size = size;
}

static const char* system_tap_sip_tx_src;
static unsigned short system_tap_sip_tx_srcPort;
static const char* system_tap_sip_tx_dst;
static unsigned short system_tap_sip_tx_dstPort;
static const char* system_tap_sip_tx_sipMessage;
static int system_tap_sip_tx_size;

void system_tap_sip_tx(
  const char* src,
  unsigned short srcPort,
  const char* dst,
  unsigned short dstPort,
  const char* sipMessage,
  int size)
{
  system_tap_sip_tx_src = src;
  system_tap_sip_tx_srcPort = srcPort;
  system_tap_sip_tx_dst = dst;
  system_tap_sip_tx_dstPort = dstPort;
  system_tap_sip_tx_sipMessage = sipMessage;
  system_tap_sip_tx_size = size;
}

static intptr_t system_tap_sip_msg_created_pointerAddress;
void system_tap_sip_msg_created(intptr_t pointerAddress)
{
  system_tap_sip_msg_created_pointerAddress = pointerAddress;
}

static intptr_t system_tap_sip_msg_destroyed_pointerAddress;
void system_tap_sip_msg_destroyed(intptr_t pointerAddress)
{
  system_tap_sip_msg_destroyed_pointerAddress = pointerAddress;
}
