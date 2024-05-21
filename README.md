# wsfStS (Introduction in English)
- a self-made C++ card game
- CLI thus hasn't much functions (status of players  or  props)
- You can add some more cards to the game by adding them to the wsfStS.cpp file. (temporarily no guide)
## todo list
- [X] single player mode
  - [X] deck importing
  - [ ] more usual cards!
- [ ] multi player mode
  - [ ] deck importing
  - [ ] multi player cards (like counter cards and draw-preventing cards) (<- not Ash Blossom & Joyous Spring)
  - [ ] program for server
## about deck importing
- You can import your own deck by creating a text file (deck.txt in the same directory with wsfStS.exe) with the following format:
  - a number N on the first line, representing the number of cards in the deck
  - Then N lines, and each line should only contain the name of the card (no space in front of or behind the name, and use underscores instead of spaces), representing the card.
  - a final blank line represents the end of the deck
- The program doesn't check if the deck is valid or not, so make sure to follow the format correctly.

# wsfStS（中文介绍）
- 自制C++卡牌游戏
- CLI界面，从而较原本少了道具，以及部分人物状态
- 你可以在wsfStS.cpp（源代码）中添加更多卡牌（，但暂时没有引导）。
## 待办列表
- [X] 单人模式
  - [X] 卡组导入
  - [ ] 更多的常见卡！
- [ ] 联机模式
  - [ ] 卡组导入
  - [ ] 多人模式特有卡（比如康、阻止抽卡等）（<-不是指灰流丽）
  - [ ] 服务端程序
## 关于卡组导入
- 你可以通过创建一个文本文件（deck.txt，且与wsfStS.exe放置于同一目录）来导入自己的卡组，文件格式如下：
  - 第一行是一个数字N，表示卡组中有多少张牌
  - 之后N行，每行表示一张牌，仅包含牌名（无前后空格，并使用下划线代替空格）
  - 最后空一行表示卡组结束
- 程序不会检查卡组是否有效，请务必按照格式正确填写。
