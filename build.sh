#!/bin/bash -ex
mkdir -p /opt/llvm/llvm-3.7.1.src/NATIVE
pushd /opt/llvm/llvm-3.7.1.src/NATIVE
/usr/bin/cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=/opt/llvm/NATIVE.cmake /opt/llvm/llvm-3.7.1.src
/usr/bin/cmake --build . --target llvm-tblgen --config Release
popd
pushd /opt/llvm/llvm-3.7.1.src/include/llvm/IR
/opt/llvm/llvm-3.7.1.src/NATIVE/bin/llvm-tblgen -gen-intrinsic -I /opt/llvm/llvm-3.7.1.src/include/llvm/IR -I /opt/llvm/llvm-3.7.1.src/lib/Target -I /opt/llvm/llvm-3.7.1.src/include /opt/llvm/llvm-3.7.1.src/include/llvm/IR/Intrinsics.td -o /opt/llvm/llvm-3.7.1.src/include/llvm/IR/Intrinsics.gen.tmp
/usr/bin/cmake -E copy_if_different /opt/llvm/llvm-3.7.1.src/include/llvm/IR/Intrinsics.gen.tmp /opt/llvm/llvm-3.7.1.src/include/llvm/IR/Intrinsics.gen
popd
pushd /opt/llvm/llvm-3.7.1.src/lib/Target/X86
/opt/llvm/llvm-3.7.1.src/NATIVE/bin/llvm-tblgen -gen-register-info -I /opt/llvm/llvm-3.7.1.src/lib/Target/X86 -I /opt/llvm/llvm-3.7.1.src/lib/Target -I /opt/llvm/llvm-3.7.1.src/include /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86.td -o /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenRegisterInfo.inc.tmp
/usr/bin/cmake -E copy_if_different /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenRegisterInfo.inc.tmp /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenRegisterInfo.inc
/opt/llvm/llvm-3.7.1.src/NATIVE/bin/llvm-tblgen -gen-disassembler -I /opt/llvm/llvm-3.7.1.src/lib/Target/X86 -I /opt/llvm/llvm-3.7.1.src/lib/Target -I /opt/llvm/llvm-3.7.1.src/include /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86.td -o /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenDisassemblerTables.inc.tmp
/usr/bin/cmake -E copy_if_different /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenDisassemblerTables.inc.tmp /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenDisassemblerTables.inc
/opt/llvm/llvm-3.7.1.src/NATIVE/bin/llvm-tblgen -gen-instr-info -I /opt/llvm/llvm-3.7.1.src/lib/Target/X86 -I /opt/llvm/llvm-3.7.1.src/lib/Target -I /opt/llvm/llvm-3.7.1.src/include /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86.td -o /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenInstrInfo.inc.tmp
/usr/bin/cmake -E copy_if_different /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenInstrInfo.inc.tmp /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenInstrInfo.inc
/opt/llvm/llvm-3.7.1.src/NATIVE/bin/llvm-tblgen -gen-asm-writer -I /opt/llvm/llvm-3.7.1.src/lib/Target/X86 -I /opt/llvm/llvm-3.7.1.src/lib/Target -I /opt/llvm/llvm-3.7.1.src/include /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86.td -o /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenAsmWriter.inc.tmp
/usr/bin/cmake -E copy_if_different /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenAsmWriter.inc.tmp /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenAsmWriter.inc
/opt/llvm/llvm-3.7.1.src/NATIVE/bin/llvm-tblgen -gen-asm-writer -asmwriternum=1 -I /opt/llvm/llvm-3.7.1.src/lib/Target/X86 -I /opt/llvm/llvm-3.7.1.src/lib/Target -I /opt/llvm/llvm-3.7.1.src/include /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86.td -o /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenAsmWriter1.inc.tmp
/usr/bin/cmake -E copy_if_different /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenAsmWriter1.inc.tmp /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenAsmWriter1.inc
/opt/llvm/llvm-3.7.1.src/NATIVE/bin/llvm-tblgen -gen-asm-matcher -I /opt/llvm/llvm-3.7.1.src/lib/Target/X86 -I /opt/llvm/llvm-3.7.1.src/lib/Target -I /opt/llvm/llvm-3.7.1.src/include /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86.td -o /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenAsmMatcher.inc.tmp
/usr/bin/cmake -E copy_if_different /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenAsmMatcher.inc.tmp /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenAsmMatcher.inc
/opt/llvm/llvm-3.7.1.src/NATIVE/bin/llvm-tblgen -gen-dag-isel -I /opt/llvm/llvm-3.7.1.src/lib/Target/X86 -I /opt/llvm/llvm-3.7.1.src/lib/Target -I /opt/llvm/llvm-3.7.1.src/include /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86.td -o /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenDAGISel.inc.tmp
/usr/bin/cmake -E copy_if_different /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenDAGISel.inc.tmp /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenDAGISel.inc
/opt/llvm/llvm-3.7.1.src/NATIVE/bin/llvm-tblgen -gen-fast-isel -I /opt/llvm/llvm-3.7.1.src/lib/Target/X86 -I /opt/llvm/llvm-3.7.1.src/lib/Target -I /opt/llvm/llvm-3.7.1.src/include /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86.td -o /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenFastISel.inc.tmp
/usr/bin/cmake -E copy_if_different /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenFastISel.inc.tmp /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenFastISel.inc
/opt/llvm/llvm-3.7.1.src/NATIVE/bin/llvm-tblgen -gen-callingconv -I /opt/llvm/llvm-3.7.1.src/lib/Target/X86 -I /opt/llvm/llvm-3.7.1.src/lib/Target -I /opt/llvm/llvm-3.7.1.src/include /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86.td -o /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenCallingConv.inc.tmp
/usr/bin/cmake -E copy_if_different /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenCallingConv.inc.tmp /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenCallingConv.inc
/opt/llvm/llvm-3.7.1.src/NATIVE/bin/llvm-tblgen -gen-subtarget -I /opt/llvm/llvm-3.7.1.src/lib/Target/X86 -I /opt/llvm/llvm-3.7.1.src/lib/Target -I /opt/llvm/llvm-3.7.1.src/include /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86.td -o /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenSubtargetInfo.inc.tmp
/usr/bin/cmake -E copy_if_different /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenSubtargetInfo.inc.tmp /opt/llvm/llvm-3.7.1.src/lib/Target/X86/X86GenSubtargetInfo.inc
popd
