import os
import sys
from pathlib import Path

# 将 PyQt6 的插件目录添加到环境变量，确保程序能找到它们
if hasattr(sys, '_MEIPASS'):
    base_path = Path(sys._MEIPASS)
    qt_plugin_dir = base_path / 'PyQt6' / 'Qt6'
    os.environ['QT_PLUGIN_PATH'] = str(qt_plugin_dir)