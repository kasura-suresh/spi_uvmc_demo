`timescale 1ns/1ns;
`include "uvm_macros.svh"
import uvm_pkg::*;
import uvmc_pkg::*;
`include "uvmc_host.svh";

module sv_main;
  
  uvmc_host uvmc_h = new("uvmc_h");
  
  initial begin
    
    uvmc_tlm#()::connect(uvmc_h.initSocket, "host");
    
    run_test();
	
  end
endmodule
