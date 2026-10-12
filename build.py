from pathlib import Path
import subprocess,concurrent.futures,json,os
root=Path(__file__).resolve().parent
repo=Path('C:/Users/Yahir Aquino/Documents/Codex/XenonRecomp')
tool=Path('C:/Users/Yahir Aquino/Documents/Codex/tools/llvm-mingw-20261006-ucrt-x86_64/bin')
os.environ['PATH']=str(tool)+os.pathsep+os.environ['PATH']
build=root/'build';build.mkdir(exist_ok=True)
bin=root/'bin';bin.mkdir(exist_ok=True)
flags=['-std=c++17','-O0','-mavx2','-msse4.1','-I',str(root/'ppc'),'-I',str(repo/'thirdparty/simde'),'-I',str(repo/'XenonUtils')]
def run(args):subprocess.run([str(x) for x in args],check=True)
header_time=max(p.stat().st_mtime for p in (root/'ppc').glob('*.h'))
runtime_header_time=max(p.stat().st_mtime for p in (root/'runtime').glob('*.h'))
def compile(p):
 hooked=p.parent.name=='ppc' and 'PPC_MM_STORE_U32' in p.read_text()
 extra=['-include',str(root/'runtime/mmio.h')] if hooked else []
 dependency=(root/'runtime/mmio.h').stat().st_mtime if hooked else 0
 obj=build/(p.stem+'.obj')
 if obj.exists() and obj.stat().st_mtime>=max(dependency,p.stat().st_mtime,header_time,runtime_header_time if p.parent.name=="runtime" else 0):return obj
 pending=obj.with_suffix('.pending.obj')
 run([tool/'clang++.exe',*flags,*extra,'-c',p,'-o',pending])
 pending.replace(obj)
 return obj
files=sorted((root/'ppc').glob('*.cpp'))
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
 objects=[]
 for obj in pool.map(compile,files):
  objects.append(obj)
  if len(objects)%32==0:print(f'Compiled or verified {len(objects)}/{len(files)}',flush=True)
(root/'build-status.json').write_text(json.dumps({'files':len(files),'passed':len(objects),'failed':[],'mode':'incremental with source/header timestamps'},indent=2))
rsp=build/'objects.rsp';rsp.write_text('\n'.join('"'+str(p).replace('\\','/')+'"' for p in objects))
run([tool/'llvm-ar.exe','rcs',bin/'DeadSpacePPC.a','@'+str(rsp)])
main=compile(root/'runtime/main.cpp');imports=compile(root/'runtime/imports.cpp');memory=compile(root/'runtime/memory.cpp');sync=compile(root/'runtime/sync.cpp');video=compile(root/'runtime/video.cpp');tls=compile(root/'runtime/tls.cpp');objects=compile(root/'runtime/objects.cpp');physical=compile(root/'runtime/physical.cpp');threads=compile(root/'runtime/threads.cpp');timestamp=compile(root/'runtime/timestamp.cpp');gpu=compile(root/'runtime/gpu.cpp');notifications=compile(root/'runtime/notifications.cpp')
run([tool/'clang++.exe',main,imports,memory,sync,video,tls,objects,physical,threads,timestamp,gpu,notifications,bin/'DeadSpacePPC.a',repo/'out/build/portable-release/XenonUtils/libXenonUtils.a',repo/'out/build/portable-release/thirdparty/disasm/libdisasm.a','-o',bin/'DeadSpaceDiagnostic.exe'])
run([tool/'clang++.exe',*flags,root/'tests/instruction-tests.cpp','-o',bin/'InstructionTests.exe'])
run([bin/'InstructionTests.exe'])
run([tool/'clang++.exe',*flags,root/'tests/memory-tests.cpp',memory,'-o',bin/'MemoryTests.exe'])
run([bin/'MemoryTests.exe'])
run([tool/'clang++.exe',*flags,root/'tests/startup-tests.cpp',imports,'-o',bin/'StartupTests.exe'])
run([bin/'StartupTests.exe'])
run([tool/'clang++.exe',*flags,root/'tests/sync-tests.cpp',imports,sync,'-o',bin/'SyncTests.exe'])
run([bin/'SyncTests.exe'])
run([tool/'clang++.exe',*flags,root/'tests/video-tests.cpp',video,'-o',bin/'VideoTests.exe'])
run([bin/'VideoTests.exe'])
r=subprocess.run([str(bin/'VideoTests.exe'),'gamma'],capture_output=True,text=True)
if r.returncode!=22:raise RuntimeError('Invalid gamma output pointer accepted')

run([tool/'clang++.exe',*flags,root/'tests/gpu-tests.cpp',gpu,'-o',bin/'GpuTests.exe'])
run([bin/'GpuTests.exe'])
for case,expected in [('event',20),('pointer',22)]:
 r=subprocess.run([str(bin/'GpuTests.exe'),case],capture_output=True,text=True)
 if r.returncode!=expected:raise RuntimeError('Graphics notification validation failed: '+case)

run([tool/'clang++.exe',*flags,root/'tests/gpu-ring-tests.cpp',gpu,'-o',bin/'GpuRingTests.exe'])
run([bin/'GpuRingTests.exe'])
run([tool/'clang++.exe',*flags,root/'tests/gpu-processor-tests.cpp',gpu,'-o',bin/'GpuProcessorTests.exe'])
run([bin/'GpuProcessorTests.exe'])
for case,expected in [('unknown',20),('truncated',22),('index',22),('cycle',22),('register',20)]:
 r=subprocess.run([str(bin/'GpuProcessorTests.exe'),case],capture_output=True,text=True)
 if r.returncode!=expected:raise RuntimeError('GPU processor rejection failed: '+case)

for case in ['shift','alignment','unmapped','overflow','order','wbshift','wbunmapped']:
 r=subprocess.run([str(bin/'GpuRingTests.exe'),case],capture_output=True,text=True)
 if r.returncode!=(20 if case=='order' else 22):raise RuntimeError('GPU ring invalid case failed: '+case)

run([tool/'clang++.exe',*flags,root/'tests/notification-tests.cpp',notifications,'-o',bin/'NotificationTests.exe'])
run([bin/'NotificationTests.exe'])
r=subprocess.run([str(bin/'NotificationTests.exe'),'invalid'],capture_output=True,text=True)
if r.returncode!=22:raise RuntimeError('Invalid notification record was accepted')
run([tool/'clang++.exe',*flags,root/'tests/tls-tests.cpp',tls,'-o',bin/'TlsTests.exe'])
run([bin/'TlsTests.exe'])
run([tool/'clang++.exe',*flags,root/'tests/thread-tests.cpp',threads,objects,tls,'-o',bin/'ThreadTests.exe'])
run([bin/'ThreadTests.exe'])
run([tool/'clang++.exe',*flags,root/'tests/thread-name-tests.cpp',imports,'-o',bin/'ThreadNameTests.exe'])
run([bin/'ThreadNameTests.exe'])
for case in ['code','pointer']:
 r=subprocess.run([str(bin/'ThreadNameTests.exe'),case],capture_output=True,text=True)
 if r.returncode!=20:raise RuntimeError('Unsupported guest exception was swallowed')
run([tool/'clang++.exe',*flags,root/'tests/clock-tests.cpp',imports,'-o',bin/'ClockTests.exe'])
run([bin/'ClockTests.exe'])
run([tool/'clang++.exe',*flags,root/'tests/timestamp-tests.cpp',timestamp,'-o',bin/'TimestampTests.exe'])
run([bin/'TimestampTests.exe'])
run([tool/'clang++.exe',*flags,root/'tests/object-tests.cpp',objects,'-o',bin/'ObjectTests.exe'])
run([bin/'ObjectTests.exe'])
run([tool/'clang++.exe',*flags,root/'tests/event-tests.cpp',objects,'-o',bin/'EventTests.exe'])
run([bin/'EventTests.exe'])
run([tool/'clang++.exe',*flags,root/'tests/kernel-event-tests.cpp',objects,'-o',bin/'KernelEventTests.exe'])
run([bin/'KernelEventTests.exe'])
r=subprocess.run([str(bin/'KernelEventTests.exe'),'unsupported'],capture_output=True,text=True)
if r.returncode!=20:raise RuntimeError('Unsupported dispatcher type was accepted')

run([tool/'clang++.exe',*flags,root/'tests/duplicate-tests.cpp',objects,'-o',bin/'DuplicateTests.exe'])
run([bin/'DuplicateTests.exe'])
run([tool/'clang++.exe',*flags,root/'tests/reference-tests.cpp',objects,'-o',bin/'ReferenceTests.exe'])
run([bin/'ReferenceTests.exe'])
run([tool/'clang++.exe',*flags,root/'tests/priority-tests.cpp',objects,'-o',bin/'PriorityTests.exe'])
run([bin/'PriorityTests.exe'])
r=subprocess.run([str(bin/'PriorityTests.exe'),'invalid'],capture_output=True,text=True)
if r.returncode!=20:raise RuntimeError('Unknown thread priority operation was not stopped')

r=subprocess.run([str(bin/'ReferenceTests.exe'),'invalid'],capture_output=True,text=True)
if r.returncode!=22:raise RuntimeError('Unbalanced object release was not rejected')
run([tool/'clang++.exe',*flags,root/'tests/physical-tests.cpp',physical,'-o',bin/'PhysicalTests.exe'])
run([bin/'PhysicalTests.exe'])
r=subprocess.run([str(bin/'VideoTests.exe'),'invalid'],capture_output=True,text=True)
if r.returncode!=22:raise RuntimeError('Invalid video buffer was not rejected')
print(f'Compiled {len(files)} translation files, diagnostic host and tests.')
