"""Inspect captured PM4 headers without executing or marking packets complete.
The snapshot may include unwritten words: it has no verified producer boundary.
"""
import argparse,json,re
from pathlib import Path

def decode(words):
    packets=[]
    i=0
    while i<len(words):
        header=words[i];kind=header>>30
        payload=0 if header==0 or kind==2 else 2 if kind==1 else ((header>>16)&0x3fff)+1
        item={"snapshot_word":i,"header":f"0x{header:08X}","type":kind,"payload_words":payload}
        if kind==3:item["opcode"]=f"0x{(header>>8)&0x7f:02X}"
        if kind==0:item["register_index"]=f"0x{header&0x7fff:04X}"
        item["complete_in_snapshot"]=i+1+payload<=len(words)
        packets.append(item)
        if not item["complete_in_snapshot"]:break
        i+=1+payload
    return packets

if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log",type=Path);args=parser.parse_args()
    words=[int(m,16) for m in re.findall(r"GPU_RING_WORD index=\d+ value=([0-9A-F]{8})",args.log.read_text())]
    print(json.dumps({"warning":"Snapshot only; producer boundary unknown, no execution", "captured_words":len(words),"packets":decode(words)},indent=2))
