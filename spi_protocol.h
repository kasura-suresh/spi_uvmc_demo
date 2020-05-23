//Author :- Akash Kumar Gupta
//Date   :- 14-May-2020
//To Implement Serial Peripheral Interface In Functional Model

#include<systemc.h>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>
using namespace tlm;
using namespace tlm_utils;

#define LOG() cout << "@ " << sc_time_stamp() << ":: " << name() \
        << ": [" << __func__ << "]: "

class spi_if: public sc_interface  // Interface Class 
{
  public:
    virtual bool full_duplex(int MOSI, int& MISO)=0;
};

SC_MODULE(SPI_Master) //SPI_Master Module
{
  sc_port<spi_if> MOSI_IF;  // sc_port
  sc_fifo_in<int> data_host_master;

  int i=0,k=1,ret,data_read;

  void Behaviour() // Behaviour I tried to write 
  {
    LOG() << "Master has initially " << data_host_master.num_available() << " data to send" << endl;
    if (data_host_master.num_available() == 0)
      wait(data_host_master.data_written_event());
    wait(1, SC_NS);
    LOG() << "Master has " << data_host_master.num_available() << " data to send" << endl;
    while(data_host_master.num_available() > 0) 
    {
      // if(data_host_master.nb_read(data_read))
      data_read = data_host_master.read();
      {
        //cout<<data_read<<endl;
        //data_read = data_host_master.read();
        cout<<"SPI_Master Send Data To SPI_Slave Is : "<< data_read << endl;
        ret = MOSI_IF->full_duplex(data_read,k);
        cout << sc_time_stamp() << " in master send " << data_read << endl;

        cout << sc_time_stamp() << " in master receive " << ret << endl;
        i++;
        k++;
      }
      /*
      else
      {
        wait(data_host_master.data_written_event());
      }
      */
    }
    wait();
  }

  SC_CTOR(SPI_Master) 
  {
    SC_THREAD(Behaviour);
    sensitive << data_host_master.data_written();
  }
};

SC_MODULE(SPI_Slave), public spi_if //SPI_Slave Module 
{
  sc_export<spi_if> MISO_IF;   //sc_export
  sc_fifo_in<int> data_host_slave;
  int val=0,data_read=0;

  bool full_duplex(int MOSI, int& MISO)  // Implementation of Interface 
  {
    // data_read = data_host_slave.read();
    bool has_read = data_host_slave.nb_read(MISO);
    if (has_read) {
      cout<<"SPI_Slave Send Data To SPI_Master Is : "<< MISO << endl;

      cout << sc_time_stamp() << " in slave send " << MISO << endl;

      cout << sc_time_stamp() << " in slave receive " << MOSI << endl;
      return true;  
    } else {
      LOG() << "No data in slave" << endl;
      return false;
    }
  }

  SC_CTOR(SPI_Slave) 
  {
    MISO_IF.bind(*this);
  }
};

SC_MODULE(Host)
{
  sc_fifo_out<int> data_host_transmitter;
  sc_fifo_out<int> data_host_receiver;

#ifdef STAND_ALONE
  int master[5] = {11,22,33,44,55};
  int slave[5] = {12,21,13,41,51};
  void inputs() 
  {
    for(int j=0;j<5;j++)
    {
      data_host_transmitter.write(master[j]);
      wait(SC_ZERO_TIME);
      data_host_receiver.write(slave[j]);
      wait(SC_ZERO_TIME);
    }
  }
#else // using UVMC
  simple_target_socket<Host> host_sock;
  void b_transport(tlm_generic_payload& gp, sc_time& delay) {
    sc_assert(gp.get_data_length() == 1);
    if (gp.is_write()) { // data for SPI_Master module
      data_host_transmitter.write(gp.get_data_ptr()[0]);
    } else {            // data for SPI_Slave module
      sc_assert(gp.is_read());
      data_host_receiver.write(gp.get_data_ptr()[0]);
    }
  }
#endif
  SC_CTOR(Host) : host_sock("host_sock") {

#ifdef STAND_ALONE
    SC_THREAD(inputs);
#else
    host_sock.register_b_transport(this, &Host::b_transport);
#endif
  }
};


#ifdef STAND_ALONE
int sc_main(int argc,char* argv[])
{

  SPI_Master master("master");
  SPI_Slave slave("slave");

  sc_fifo<int> data_sent_to_transmitter;
  sc_fifo<int> data_sent_to_receiver;

  master.MOSI_IF(slave.MISO_IF);

  Host host("host");

  host.data_host_transmitter(data_sent_to_transmitter);
  host.data_host_receiver(data_sent_to_receiver);

  master.data_host_master(data_sent_to_transmitter);

  slave.data_host_slave(data_sent_to_receiver);



  sc_start();
  return 0;
}
#endif
