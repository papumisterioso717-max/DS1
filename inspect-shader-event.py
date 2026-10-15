"""Read-only EVENT_WRITE_SHD decoding; never signals completion."""
import json,re,sys
from pathlib import Path

def decode(initiator,address,value):
    if any(type(v) is not int or not 0<=v<=0xffffffff for v in (initiator,address,value)):
        raise ValueError('Expected unsigned 32-bit fields')
    mode=address&3
    data=list(value.to_bytes(4,'little'))
    order=((0,1,2,3),(1,0,3,2),(3,2,1,0),(2,3,0,1))[mode]
    return {'event':initiator&63,'counter_requested':bool(initiator>>31),
        'physical_address':f'0x{address&~3:08X}','endianness':mode,
        'requested_bytes':None if initiator>>31 else [f'{data[i]:02X}' for i in order],
        'completion_proven':False,'memory_written':False}

if __name__=='__main__':
    log=Path(sys.argv[1]).read_text()
    events=re.findall(r'GPU_SHADER_EVENT_CAPTURE initiator=([0-9A-F]{8}).*? address=([0-9A-F]{8}).*? value=([0-9A-F]{8}) bypassed_draws=(\d+)',log)
    if not events:raise ValueError('No shader event capture')
    print(json.dumps([dict(decode(*(int(v,16) for v in event[:3])),bypassed_draws=int(event[3])) for event in events],indent=2))
