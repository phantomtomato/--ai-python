# 五子棋 AI 项目

本项目是一个基于 Python 和 C++ 实现的五子棋游戏，其中 AI 算法参考自 [colingogogo/gobang_AI](https://github.com/colingogogo/gobang_AI)，并将其核心算法改写为 C++ 语言以提高运行效率。

## 项目背景
五子棋是一款经典的棋类游戏，为了实现一个具有一定智能水平的五子棋 AI，本项目参考了开源项目 `colingogogo/gobang_AI` 的算法思路，并在此基础上进行优化。将关键的 AI 计算部分用 C++ 实现，通过 Python 调用 C++ 编译生成的 DLL 文件，在保证功能完整性的同时显著提升了计算速度。

## 项目结构
├── 五子棋.py # Python 主程序，负责游戏界面和交互逻辑 
├── gobang_ai.dll # C++ 编译生成的动态链接库，包含 AI 核心算法 
├── SimHei.ttf # 游戏使用的字体文件 └── README.md # 项目说明文档


## 功能特性
- **图形界面**：使用 `pygame` 库创建了直观的五子棋游戏界面。
- **AI 对战**：集成了 AI 算法，玩家可以与 AI 进行对战。
- **悔棋功能**：支持按 `Z` 键进行悔棋操作。

## 运行环境
### 软件依赖
- **Python 3.x**：建议使用 Python 3.13 及以上版本。
- **pygame**：用于创建游戏界面和处理用户交互。
- **MinGW 或 Visual Studio**：用于编译 C++ 代码生成 DLL 文件。

### 安装依赖
在命令行中执行以下命令安装 Python 依赖：
```bash
pip install pygame
编译与运行
编译 C++ 代码
如果你对 C++ 代码进行了修改，需要重新编译生成 DLL 文件。

使用 MinGW

bash
g++ -shared -o gobang_ai.dll gobang_ai.cpp -std=c++17 -pthread
使用 Visual Studio
创建一个新的动态链接库 (DLL) 项目。
将 gobang_ai.cpp 代码复制到项目中。
配置项目属性，将 C++ 语言标准设置为 ISO C++17 标准 (/std:c++17)。
生成解决方案，在输出目录中找到 gobang_ai.dll 文件。
运行游戏
在命令行中进入项目目录，执行以下命令启动游戏：


bash
python 五子棋.py
玩法说明
落子：玩家执白子，通过鼠标点击棋盘交叉点进行落子。
AI 落子：玩家落子后，AI 会自动计算并落黑子。
悔棋：按 Z 键可以撤销上一步操作，但在 AI 计算过程中无法悔棋。
获胜判断：当某一方在横、竖、斜方向上有连续五个相同颜色的棋子时，该方获胜。
算法参考
本项目的 AI 算法主要参考自 colingogogo/gobang_AI，并进行了以下改进：

语言转换：将原 Python 实现的算法改写为 C++ 语言，利用 C++ 的高性能特性加快计算速度。
搜索优化：在 negamax 算法中采用 alpha - beta 剪枝技术，减少不必要的搜索，提高算法效率。
贡献与反馈
如果你对本项目有任何建议或发现了 bug，欢迎提交 issues 或 pull requests。我们非常欢迎你的贡献！
