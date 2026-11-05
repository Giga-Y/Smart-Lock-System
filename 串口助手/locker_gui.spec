# -*- mode: python ; coding: utf-8 -*-
from PyInstaller.utils.hooks import collect_all

block_cipher = None

# 自动收集 PyQt6 所有的动态库、插件和资源文件
datas, binaries, hiddenimports = collect_all('PyQt6')
# 强制补充你之前需要的隐藏导入
hiddenimports += [
    'pkgutil', 
    'PyQt6.sip', 
    'serial', 
    'serial.tools', 
    'serial.tools.list_ports'
]

a = Analysis(
    ['locker_gui.py'],
    pathex=[],
    binaries=binaries,       # 这里现在包含了 qwindows.dll
    datas=datas,             # 这里包含了插件所需的目录结构
    hiddenimports=hiddenimports,
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    win_no_prefer_redirects=False,
    win_private_assemblies=False,
    cipher=block_cipher,
    noarchive=False,
)

pyz = PYZ(a.pure, a.zipped_data, cipher=block_cipher)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.zipfiles,
    a.datas,
    [],
    name='locker_gui',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=False,           # 如果你遇到闪退，可以暂时改为 True 看看报错
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)