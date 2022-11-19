
import cocotb
from cocotb.triggers import Timer, RisingEdge
from cocotb.clock import Clock
from cocotb.handle import SimHandleBase
from random import randint

@cocotb.test()
async def sanity(dut):
    cocotb.start_soon(Clock(dut.clk, 1, units="ns").start())

    dut.resetn.value = 0
    for _ in range(4):
        await RisingEdge(dut.clk)

    dut.resetn.value = 1

    for _ in range(20):
        await RisingEdge(dut.clk)

