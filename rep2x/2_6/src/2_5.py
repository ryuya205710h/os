import os
import glob

# スクリプトのディレクトリを取得
script_directory = os.path.dirname(os.path.abspath(__file__))

# 検索したい拡張子
target_extension = ''

# ファイル検索
for file_path in glob.glob(f'{script_directory}/**/*.{target_extension}', recursive=True):
    if not os.path.isdir(file_path):
        print(file_path)
