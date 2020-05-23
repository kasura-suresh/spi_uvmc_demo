#include "uvmc.h"
using namespace uvmc;

#include "spi_protocol.h"

int sc_main( int argc, char* argv[] )
{
  SPI_Master spi_m("spi_m");
  SPI_Slave  spi_s("spi_s");
  spi_m.MOSI_IF(spi_s.MISO_IF);


  Host sc_host("sc_host");
  uvmc_connect (sc_host.host_sock,"host");

  sc_fifo<int> data_sent_to_transmitter;
  sc_fifo<int> data_sent_to_receiver;

  sc_host.data_host_transmitter(data_sent_to_transmitter);
  sc_host.data_host_receiver(data_sent_to_receiver);

  spi_m.data_host_master(data_sent_to_transmitter);
  spi_s.data_host_slave(data_sent_to_receiver);

  sc_start(-1);
  return 0;
}


