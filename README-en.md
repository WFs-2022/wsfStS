# wsfStS (Introduction in English)
- a self-made C++ card game
- CLI thus hasn't much functions (status of players  or  props)
- You can add some more cards to the game by adding them to the wsfStS.cpp file. (temporarily no guide)
## todo list
- [X] single player mode
  - [X] deck importing
  - [ ] more usual cards! (can be also provided by everyone~)
- [X] multi player mode
  - [X] deck importing
  - [X] program for server
  - [ ] multi player cards (like counter cards and draw-preventing cards) (<- not Ash Blossom & Joyous Spring)
## about deck importing
- You can import your own deck by creating a text file (deck.txt in the same directory with wsfStS.exe) with the following format:
  - a number N on the first line, representing the number of cards in the deck
  - Then N lines, and each line should only contain the name of the card (no space in front of or behind the name, and use underscores instead of spaces), representing the card.
  - a final blank line represents the end of the deck
- The program doesn't check if the deck is valid or not, so make sure to follow the format correctly.