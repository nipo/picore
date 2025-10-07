from pymodbus.client import AsyncModbusSerialClient
import asyncclick as click

@click.group()
def cli():
    pass

@cli.command()
@click.argument("port", type = str)
async def demo(port):
    client = AsyncModbusSerialClient(port)
    await client.connect()
    print("Writing coil 0 = True")
    await client.write_coil(0, True, device_id=9)
    print("Writing coil 1 = True")
    await client.write_coil(1, True, device_id=9)
    print("Reading register 0", end = "", flush = True)
    regs = await client.read_holding_registers(0, device_id=9)
    print(f" -> {regs.registers[0]:#x}")
    assert regs.registers[0] == 3
    
    print("Writing coil 0 = False")
    await client.write_coil(0, False, device_id=9)
    print("Reading register 0", end = "", flush = True)
    regs = await client.read_holding_registers(0, device_id=9)
    print(f" -> {regs.registers[0]:#x}")
    assert regs.registers[0] == 2

if __name__ == "__main__":
    cli()
    
