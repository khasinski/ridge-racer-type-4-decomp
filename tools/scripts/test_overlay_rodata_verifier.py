"""Declared overlay tables must match after linking; undeclared data remains rejected."""
from contextlib import redirect_stdout
from io import StringIO
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest.mock import patch
import json
import struct
import subprocess
import unittest
from elftools.elf.elffile import ELFFile
import verify_overlay_samples as verifier

ROOT=Path(__file__).resolve().parents[2]

class OverlayRodataVerifierTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp=TemporaryDirectory();cls.addClassCleanup(cls.tmp.cleanup)
        cls.root=Path(cls.tmp.name);cls.address=0x80114998;cls.data_address=0x801148F8
        (cls.root/'tools').symlink_to(ROOT/'tools',target_is_directory=True)
        (cls.root/'include').symlink_to(ROOT/'include',target_is_directory=True)
        cls.source=cls.root/'src/overlays/PAL/garage/func_80114998.c';cls.source.parent.mkdir(parents=True)
        cls.source.write_text(''.join(f'void func_{0x80011100+i*16:08X}(void);\n' for i in range(5))+
            'void garage_init_main_menu(int mode){switch(mode){'+''.join(f'case {i}:func_{0x80011100+i*16:08X}();break;' for i in range(5))+'default:break;}}\n')
        cls.obj=cls.root/'fixture.o';cls.elf=cls.root/'fixture.elf';ld=cls.root/'fixture.ld'
        subprocess.run([str(ROOT/'tools/scripts/cc.sh'),str(cls.source),str(cls.obj)],check=True,capture_output=True)
        ld.write_text(''.join(f'func_{0x80011100+i*16:08X} = 0x{0x80011100+i*16:X};\n' for i in range(5))+
            f'SECTIONS {{ .text 0x{cls.address:X} : SUBALIGN(4) {{ *(.text) }} .rodata 0x{cls.data_address:X} : SUBALIGN(4) {{ *(.rodata) }} }}\n')
        subprocess.run([str(ROOT/'tools/toolchain/bin/mipsel-none-elf-ld'),'-EL','-T',str(ld),'-o',str(cls.elf),str(cls.obj)],check=True,capture_output=True)
        with cls.elf.open('rb') as stream:
            parsed=ELFFile(stream);cls.code=parsed.get_section_by_name('.text').data();cls.data=parsed.get_section_by_name('.rodata').data()
        assert len(cls.data)==20
        cls.span=(cls.data_address,len(cls.data))
        module=bytearray(4096);start=cls.address-0x80114878;module[start:start+len(cls.code)]=cls.code
        start=cls.data_address-0x80114878;module[start:start+len(cls.data)]=cls.data;cls.module=bytes(module)
        blob=bytearray(8192);struct.pack_into('<I',blob,0,950);struct.pack_into('<951I',blob,4,*([2]*663+[4]*288));blob[4096:]=module;cls.blob=bytes(blob)
        (cls.root/'disc/PAL').mkdir(parents=True);(cls.root/'disc/PAL/R4.BIN').write_bytes(cls.blob)
        exe=b'PS-X EXE'+bytes(2040)
        for folder in ['assets/PAL','build/PAL']:
            (cls.root/folder).mkdir(parents=True);(cls.root/folder/'main.exe').write_bytes(exe)

    def test_object_data_requires_exact_explicit_declaration(self):
        with self.obj.open('rb') as stream:
            parsed=ELFFile(stream)
            with self.assertRaisesRegex(ValueError,'Unexpected data section'):
                verifier.validate_object_sections(parsed,self.source,None)
            verifier.validate_object_sections(parsed,self.source,self.span)
            with self.assertRaisesRegex(ValueError,'Unexpected data section'):
                verifier.validate_object_sections(parsed,self.source,(self.data_address,len(self.data)+4))

    def test_linked_data_requires_exact_contents_address_and_bounds(self):
        with self.elf.open('rb') as stream:
            parsed=ELFFile(stream)
            self.assertEqual(verifier.read_linked_rodata(parsed,self.span,self.module),self.data)
            with self.assertRaisesRegex(ValueError,'Undeclared'):
                verifier.read_linked_rodata(parsed,None,self.module)
            wrong=bytearray(self.module);wrong[self.data_address-0x80114878]^=1
            with self.assertRaisesRegex(ValueError,'does not match'):
                verifier.read_linked_rodata(parsed,self.span,wrong)
            with self.assertRaisesRegex(ValueError,'does not match its declared span'):
                verifier.read_linked_rodata(parsed,(self.data_address+4,len(self.data)),self.module)
            with self.assertRaisesRegex(ValueError,'outside the module'):
                verifier.read_linked_rodata(parsed,(0x80114874,len(self.data)),self.module)

    def test_full_pipeline_accepts_original_table_and_rejects_modified_table(self):
        def real_tool(*args):
            # cc.sh derives its project root from argv0; resolve the fixture symlink.
            subprocess.run([str(Path(args[0]).resolve()),*(str(a) for a in args[1:])],check=True,capture_output=True)
        with patch.multiple(verifier,run=real_tool,ROOT=self.root,SAMPLES=[(662,self.address,len(self.code))],EXTRA_FUNCTIONS=[],ALIASES={},RODATA_SPANS={(662,self.address):self.span}):
            (self.root/'disc/PAL/R4.BIN').write_bytes(self.blob)
            with redirect_stdout(StringIO()):verifier.main()
            report=json.loads((self.root/'build/overlays/samples/verified.json').read_text())
            self.assertEqual(report['functions'][0]['levenshtein'],0)
            self.assertEqual(report['functions'][0]['rodata'],{'address':self.data_address,'size':20,'identical':True})
            self.assertTrue(report['reconstructed_r4_identical'] and report['full_exe_identical'])
            bad=bytearray(self.blob);bad[4096+self.data_address-0x80114878]^=1;(self.root/'disc/PAL/R4.BIN').write_bytes(bad)
            with self.assertRaisesRegex(ValueError,'does not match'):
                verifier.main()
