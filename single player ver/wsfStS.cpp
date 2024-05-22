/***********************************************/
/* File Name: wsfStS.cpp (wsf Slay the Spire)  */
/* Creator: CVE-2020-0796 (bilibili)           */
/* Date: 2024/05/17                            */
/* Comment: A small simulate of StS but of CLI */
/* P.S. It's the SINGLE PLAYER version         */
/* LICENSE: it's under GPLv3 License           */
/***********************************************/

#include <bits/stdc++.h>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <cstdio>
using namespace std;

// Declare of usual useful functions, copied from my other CPP games
void init();
//POINT GetMousePos();
//void gotoxy(int x, int y);

// Card structure
struct Card {
    string name, type; // type is from {"phy"(physical), "ablt"(abilities), "mag"(magical), "psy"(psychological)}
    int cost, obj, damag, block, heal, draw, power, aglty, sheld, times; // 'times' for repeating times
//    obj: 0->choose; 1->player, and so on
    string text;
};

// Map for str2Card (deck to player.deck)
map<string, Card> cardDict = {
							  {"Strike", {"Strike", "phy", 1, 0, 5, 0, 0, 0, 0, 0, 0, 1, "Deal 5 damage"}}, 
							  {"Strikeee", {"Strikeee", "phy", 1, 0, 2, 0, 0, 0, 0, 0, 0, 3, "Deal 2 damage to a player 3 times"}}, 
							  {"Mana_hit", {"Mana hit", "phy", 2, 0, 6, 0, 0, 0, 0, 0, 0, 1, "Deal 7 damage, and you'll get 1 more mana next turn"}}, 
							  {"Crash", {"Crash", "phy", 1, 0, 3, 0, 0, 1, 0, 0, 0, 1, "Deal 3 damage and draw 1 card"}}, 
							  {"Defend", {"Defend", "phy", 1, 1, 0, 6, 0, 0, 0, 0, 0, 1, "Get 6 block"}}, 
							  {"Dodge", {"Dodge", "phy", 1, 1, 0, 4, 0, 2, 0, 0, 0, 1, "Get 4 block and draw 2 cards"}}, 
							  {"Power_up", {"Power up" , "ablt", 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, "Add 1 strength"}}, 
							  {"Armor_up", {"Armor up" , "ablt", 1, 1, 0, 0, 0, 0, 0, 2, 0, 1, "Add 2 agility"}}, 
							  {"Mana_save", {"Mana save", "mag" , 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "You'll get 2 more mana next turn"}}, 
							  {"Twice_act", {"Twice act", "mag" , 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "The next 'phy' card you use this turn will be processed twice"}}, 
							  {"Hypnotize", {"Hypnotize", "psy" , 2, 0, 0, 0, 0, 0,-1,-1, 0, 1, "Decrease a player's strength and defence for 1"}}
							 };

// Player structure
struct Player {
	string name;
    int health, block, power, aglty;
	bool sheld;
    vector<Card> deck;
    vector<Card> hand;
	vector<Card> tomb;
	vector<Card> exil;
	vector<Card> stat;
} player, enemy;

// the initial health of PVE player and enemy
int PVE_PHealth = 60, PVE_EHealth = 80;
// the name of player and PVE enemy
string playerName = "player", PVE_enemyName = "zombie";

// Function to initialize player's deck
void deckImport(Player& player) {
	cout << "* Now cleaning up the old deck...\n";
	player.deck.erase(player.deck.begin(), player.deck.end());
	cout << "* Now starting to import deck from deck.txt\n";
	if(freopen("deck.txt", "r", stdin) == NULL){
		// no file for import then use startup deck		
		freopen("CON", "r", stdin);
		cout<<"* No deck found. pls put a deck.txt under the folder of this program if importing. Now using the startup deck.\n";
		player.deck.push_back({"Strike"   , "phy" , 1, 0, 5, 0, 0, 0, 0, 0, 0, 1, "Deal 5 damage"});
		player.deck.push_back({"Strike"   , "phy" , 1, 0, 5, 0, 0, 0, 0, 0, 0, 1, "Deal 5 damage"});
		player.deck.push_back({"Strike"   , "phy" , 1, 0, 5, 0, 0, 0, 0, 0, 0, 1, "Deal 5 damage"});
		player.deck.push_back({"Strike"   , "phy" , 1, 0, 5, 0, 0, 0, 0, 0, 0, 1, "Deal 5 damage"});
		player.deck.push_back({"Strike"   , "phy" , 1, 0, 5, 0, 0, 0, 0, 0, 0, 1, "Deal 5 damage"});
		player.deck.push_back({"Mana hit" , "phy" , 2, 0, 6, 0, 0, 0, 0, 0, 0, 1, "Deal 7 damage, and you'll get 1 more mana next turn"});
		player.deck.push_back({"Strikeee" , "phy" , 1, 0, 2, 0, 0, 0, 0, 0, 0, 3, "Deal 2 damage to a player 3 times"});
		player.deck.push_back({"Crash"    , "phy" , 1, 0, 3, 0, 0, 1, 0, 0, 0, 1, "Deal 3 damage and draw 1 card"});
		player.deck.push_back({"Defend"   , "phy" , 1, 1, 0, 6, 0, 0, 0, 0, 0, 1, "Get 6 block"});
		player.deck.push_back({"Defend"   , "phy" , 1, 1, 0, 6, 0, 0, 0, 0, 0, 1, "Get 6 block"});
		player.deck.push_back({"Defend"   , "phy" , 1, 1, 0, 6, 0, 0, 0, 0, 0, 1, "Get 6 block"});
		player.deck.push_back({"Defend"   , "phy" , 1, 1, 0, 6, 0, 0, 0, 0, 0, 1, "Get 6 block"});
		player.deck.push_back({"Defend"   , "phy" , 1, 1, 0, 6, 0, 0, 0, 0, 0, 1, "Get 6 block"});
		player.deck.push_back({"Dodge"    , "phy" , 1, 1, 0, 4, 0, 2, 0, 0, 0, 1, "Get 4 block and draw 2 cards"});
		player.deck.push_back({"Power up" , "ablt", 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, "Add 1 strength"});
		player.deck.push_back({"Power up" , "ablt", 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, "Add 1 strength"});
		player.deck.push_back({"Armor up" , "ablt", 1, 1, 0, 0, 0, 0, 0, 2, 0, 1, "Add 2 agility"});
		player.deck.push_back({"Mana save", "mag" , 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "You'll get 2 more mana next turn"});
		player.deck.push_back({"Twice act", "mag" , 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "The next 'phy' card you use this turn will be processed twice"});
		player.deck.push_back({"Hypnotize", "psy" , 2, 0, 0, 0, 0, 0,-1,-1, 0, 1, "Decrease a player's strength and defence for 1"});
	} else {
		// importing
		int deckSize;
		cin >> deckSize;
		string cardName;
		for (int i = 0; i < deckSize; i++){
			cin >> cardName;
			player.deck.push_back(cardDict[cardName]); 
		}
		freopen("CON", "r", stdin);
	}
	cout << "* Deck loading complete.\n";
}

// Function to print the player's deck (player.deck)
void deckCheck(Player& player){
	for (unsigned int i = 0; i < player.deck.size(); i++)
		cout << player.deck[i].name << endl;
	return;
} 
// Function to show all cards involved by this program
void cardsShow(){
	cout << "All cards can be get are: " << endl << \
	"Strike		phy	Deal 5 damage" << endl <<\
	"Strikeee	phy	Deal 2 damage to a player 3 times" << endl <<\
	"Mana hit	phy	Deal 7 damage, and you'll get 1 more mana next turn" << endl <<\
	"Crash		phy	Deal 3 damage and draw 1 card" << endl <<\
	"Defend		phy	Get 6 block" << endl <<\
	"Dodge		phy	Get 4 block and draw 2 cards" << endl <<\
	"Power up	ablt	Add 1 strength" << endl <<\
	"Armor up	ablt	Add 2 agility" << endl <<\
	"Mana save	mag	You'll get 2 more mana next turn" << endl <<\
	"Twice act	mag	The next 'phy' card you use this turn will be processed twice" << endl <<\
	"Hypnotize	psy	Decrease a player's strength and defence for 1" << endl;
	return;
}

// Function to shuffle the deck
void shuffleDeck(Player& player){
	int cardNum = player.deck.size();
	for(int i = 0; i < cardNum; i++){
		srand(int(clock()*1000));
		std::vector<Card>::size_type select = rand() % (cardNum-i);
		player.deck.push_back(player.deck[select]);
		player.deck.erase(player.deck.begin() + select);
	}
}

// Function of drawing cards, shuffle GY into deck when no card in deck and dont draw with a full hand
void drawP(Player& player, unsigned int num){
	for(unsigned int i = 0; i < num; i++){
		if(player.deck.size() == 0){
			// doing as StS
			cout<<"\n* No cards in the deck, shuffling the GY into the deck...\n";
			player.deck = player.tomb;
			player.tomb.erase(player.tomb.begin(), player.tomb.end());
			shuffleDeck(player);
		} else if(player.hand.size() >= 10){
			// also doing as StS, not sending the card into the gy like HS :)
			cout<<"Your hand is too full!\n";
			return;
		}
		// get card into hand and delete it from the deck
	    player.hand.push_back(player.deck.back());
	    player.deck.pop_back();
	}
}

// Function to handle battling things
void battleP(Player& obj, int damage){
	obj.block -= damage;
	if (obj.block < 0) {
		obj.health += obj.block;
		obj.block = 0;
	}
	return;
}

// Function to simulate a duel between player and computer
void duelPVE(Player& player, Player& enemy) {
	// initialization of the duel
	player = {playerName, PVE_PHealth, 0, 0, 0, 0, player.deck, {}, {}, {}, {}};
	enemy = {PVE_enemyName, PVE_EHealth, 0, 0, 0, 0, {}, {}, {}, {}, {}};
	shuffleDeck(player);
	player.health = PVE_PHealth;
	enemy.health = PVE_EHealth;
	int mana = 3, savedMana = 0, drawNum = 5, Twice_act = 0;
	int enemyAction, enemyActionPara[3]; // enemyAction: 001: attack; 010: block; 100: power up 1;; MAX: 111 = 7
	
	while (player.health > 0 && enemy.health > 0) {
		// Decide the action of the enemy
		srand(int(clock()*1000));
		enemyAction = rand()%7 + 1;
		srand(int(clock()*1000));
		enemyActionPara[0] = rand()%4 + 6;
		srand(int(clock()*1000));
		enemyActionPara[1] = rand()%7 + 8;
		srand(int(clock()*1000));
		enemyActionPara[2] = rand()%1 + 1;
		
		// Player's turn
		cout << "\n- Player's turn:" << endl;
		drawP(player, drawNum);
		drawNum = 5;
		while(1){
			// Part II of the turn initialization: enemy action
			cout<<"! The "<< enemy.name << " is going to:\n";
        	int tmpEA = enemyAction;
			if(tmpEA%2) cout << "deal " << enemyActionPara[0] << " damage to you; ";
			tmpEA/=2;
			if(tmpEA%2) cout << "get " << enemyActionPara[1] << " block; ";
			tmpEA/=2;
			if(tmpEA%2) cout << "power up by " << enemyActionPara[2] << " points;";
			// Status outputing
			cout << "\n* Now, " << player.name << " has " << player.health << " heath, " << player.block << " block, " << player.power << " strength, and " << player.aglty << " agility;";
			cout << "\n while " << enemy.name << " has " << enemy.health << " heath, " << enemy.block << " block, " << enemy.power << " strength, and " << enemy.aglty << " agility;";
			cout << "\n* Size of your deck: " << player.deck.size() << " | grave yard: " << player.tomb.size() << " | exile: " << player.exil.size() << ";";
			cout << "\nWith " << mana << " mana left, choose a card to play: \n(  name		cost	type	content)\n0. End turn	/	/	/ \n";
			// hand display
			for (unsigned int i = 0; i < player.hand.size(); ++i)
				cout << i + 1 << ". " << player.hand[i].name << "	" << player.hand[i].cost << "	" << player.hand[i].type << "	" << player.hand[i].text << endl;
			unsigned int choice;
			cin >> choice;
			// card choice processing
			if (choice < 0 || choice > player.hand.size()) {
				// out of cards/controller (0)
				cout << "><Invalid choice!" << endl;
			} else if (choice == 0){
				// go to EP
				cout << "- Your turn ended.\n";
				break;
			} else if(player.hand[choice - 1].cost > mana) {
				cout << "><You don't have enough mana for that!\n";
			} else {
				// waiting the player for choosing the card to play
				Card playCard = player.hand[choice - 1];
				int objNum;
				if(playCard.obj == 0){
					cout << "? Play the card " << playCard.name << " to: (1 for yourself, 2 for the enemy)\n";
					while(1){
						cin >> objNum;
						if(objNum != 1 && objNum != 2){
							cout << "><Object not found, pls retry.\n";
						} else break;
					}
				} else objNum = playCard.obj;
				// the cost is always processed (and always processed first)
				mana -= playCard.cost;
				// processing card effects
				cout << "! You played " << playCard.name << endl;
				if (playCard.name == "Mana save"){ // Special processing cards
					savedMana += 2;
					cout<<"! 2 mana saved for next turn by 'Mana save'.\n";
				} else if (playCard.name == "Twice act"){
					Twice_act = 1;
				} else if (playCard.name == "Mana hit"){
					Player *object;
					if(objNum == 1) object = &player;
					else if(objNum == 2) object = &enemy;
					battleP(*object, max(playCard.damag + player.power, 0));
					cout << playCard.name << " dealt " << max(playCard.damag + player.power, 0) << " damage to " << object->name << ";" << endl;
					savedMana += 1;
					cout<<"! 1 mana saved for next turn by 'Mana hit'.\n";
					if(Twice_act){
						cout << "- Effect of 'Twice act':\n";
						battleP(*object, max(playCard.damag + player.power, 0));
						cout << playCard.name << " dealt " << max(playCard.damag + player.power, 0) << " damage to " << object->name << ";" << endl;
						savedMana += 1;
						cout<<"! 1 mana saved for next turn by 'Mana hit'.\n";
						Twice_act = 0;
					}
				} else {                           // usual cards whose effects are instant
					Player *object;
					if(objNum == 1) object = &player;
					else if(objNum == 2) object = &enemy;
					for(int i = 0; i < playCard.times; i++){
						if(playCard.damag != 0){
							battleP(*object, max(playCard.damag + player.power, 0));
							cout << playCard.name << " dealt " << max(playCard.damag + player.power, 0) << " damage to " << object->name << ";" << endl;
						}
						if(playCard.block != 0) object->block += max(playCard.block + object->aglty, 0);
						object->health += playCard.heal; // Not limited, the same as YGO and MTG
						if(playCard.draw != 0) drawP(player, playCard.draw);
						object->power += playCard.power;
						object->aglty += playCard.aglty;
						player.sheld = (player.sheld || playCard.sheld);
					}
					if(Twice_act){
						cout << "- Effect of 'Twice act':\n";
						// here i just copied the code upside
						for(int i = 0; i < playCard.times; i++){
							if(playCard.damag != 0){
								battleP(*object, max(playCard.damag + player.power, 0));
								cout << playCard.name << " dealt " << max(playCard.damag + player.power, 0) << " damage to " << object->name << ";" << endl;
							}
							object->block += max(playCard.block + object->aglty, 0);
							object->health += playCard.heal;
							if(playCard.draw) drawP(player, playCard.draw);
							object->power += playCard.power;
							object->aglty += playCard.aglty;
							player.sheld = (player.sheld || playCard.sheld);
						}
						// till here, copied the code to process the card (again)
						Twice_act = 0;
					}
				}
				// used cards should be thrown into the gy except for ability cards which should be exiled
				if(playCard.type != "ablt") player.tomb.push_back(playCard);
				else player.exil.push_back(playCard);
				std::vector<Card>::size_type select = choice - 1;
				player.hand.erase(player.hand.begin() + select);
			}
			
			if (enemy.health <= 0) {
				cout << enemy.name << " is defeated!" << endl;
				return;
			}
		}
		
		// End turn: throw all cards in hand & clear enemy's block & 'Twice act' effect ends
		while(player.hand.size()){
			player.tomb.push_back(player.hand.back());
			player.hand.pop_back();
		}
		enemy.block = 0;
		Twice_act = 0;
		
		// Enemy's turn
		cout << endl << "- " << enemy.name << "'s turn:\n" << endl;
		Sleep(800);
		
		if(enemyAction%2){
			cout << "! " << enemy.name << " dealt " << max(enemyActionPara[0] + enemy.power, 0) << " damage to you.\n";
			battleP(player, max(enemyActionPara[0] + enemy.power, 0));
			cout << "* Now you have " << player.health << " health, and " << player.block <<" block;\n";
		}
		if (player.health <= 0) { // only need to check it here as no more damage dealt in the later actions of the enemy
			cout << "You were defeated by " << enemy.name << "!" << endl;
			break;
		} enemyAction/=2;
		if(enemyAction%2){
			cout << "! " << enemy.name << " gets " << max(enemyActionPara[1] + enemy.aglty, 0) << " block.\n";
			enemy.block += max(enemyActionPara[1] + enemy.aglty, 0);
			cout << "* Now " << enemy.name << " has " << enemy.health << " health and " << enemy.block << " block;\n";
		} enemyAction/=2;
		if(enemyAction%2){
			cout << "! " << enemy.name << " gets " << enemyActionPara[2] << " more strength and " << enemyActionPara[2] << " more agility.\n";
			enemy.power += enemyActionPara[2];
			enemy.aglty += enemyActionPara[2];
			cout << "* Now " << enemy.name << " has " << enemy.power << " strength and " << enemy.aglty << " agility.\n";
		}
		Sleep(3000);

		
		// End turn; Next turn
		player.block = 0; // BLOCK OVER
        mana = 3 + savedMana; // player mana reset
        savedMana = 0;
		cout << "\n- " << enemy.name << "'s turn ended;\n";
    }
    return;
}

int main() {
	init();
	cout << "Welcome to the wsfStS! \n* Automatically importing the deck...\n";

	deckImport(player);

	string input;
	while(1){
		// main menu 
		cout << "\nPls select a mode to play(input the number):\n";
		cout << "1. PVE mode\n2. settings\n3. about\n4. exit the game\n";
		cin >> input;
		if (input == "1"){
			// single player
			duelPVE(player, enemy);
			player.health = PVE_PHealth;
			enemy.health = PVE_EHealth;
			cout << "PVE mode game over!\n";
		}else if (input == "2"){
			// settings menu
			while (1) {
				cout << "\nSettings:\n";
				cout << "1. Change player name (" << playerName << ")\n";
				cout << "2. Set PVE mode enemy name (" << PVE_enemyName << ")\n";
				cout << "3. Set PVE mode player health (" << PVE_PHealth << ")\n";
				cout << "4. Set PVE mode enemy health (" << PVE_EHealth << ")\n";
				cout << "5. Check your deck\n";
				cout << "6. Import a deck\n";
				cout << "7. Show all cards\n";
				cout << "8. Quit Settings\n";
				cin >> input;
				if(input == "1"){
					cout << "Input the new player name: ";
					cin >> playerName;
				} else if(input == "2"){
					cout << "Input the new enemy name: ";
					cin >> PVE_enemyName;
				} else if(input == "3"){
					cout << "Input the new player health: ";
					cin >> PVE_PHealth;
				} else if(input == "4"){
					cout << "Input the new enemy health: ";
					cin >> PVE_EHealth;
				} else if(input == "5"){
					deckCheck(player);
				} else if(input == "6"){
					deckImport(player);
				} else if(input == "7"){
					cardsShow();
				} else if(input == "8"){
					break;
				} else {
					cout << "* Invalid choice, pls try again.\n";
				}
			}
		} else if (input == "3"){
			cout << "A small simulate of StS but of CLI.\n";
			cout << "Made by CVE-2020-0796 on 18th May, 2024.\n";
			cout << "bilibili:  https://space.bilibili.com/518624115\n";
			cout << "YouTube:  https://www.youtube.com/channel/UCazOP122G3Yo0JXFjuAfTlQ\n";
			cout << "website:  https://wfs-2022.github.io/myNewWebsite/\n";
		} else if (input == "4") break;
		else cout << "* Invalid choice, pls try again.\n";
	}
	return 0;
}




void init(){
	system("title Slay the Spire?");
//	CONSOLE_CURSOR_INFO cursor_info = {1, 0};
//	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
	char cmd[64];
	sprintf(cmd, "mode con cols=%d lines=%d", 60, 40);
//	system(cmd);
//	SetWindowLongPtrA(GetConsoleWindow(), GWL_STYLE, GetWindowLongPtrA(GetConsoleWindow(),GWL_STYLE)& ~WS_SIZEBOX & ~WS_MAXIMIZEBOX & ~WS_MINIMIZEBOX);
}
//POINT GetMousePos(){
//	POINT p;
//	GetCursorPos(&p);
//	ScreenToClient(GetForegroundWindow(),&p);
//	CONSOLE_FONT_INFO consoleCurrentFont;
//	GetCurrentConsoleFont(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &consoleCurrentFont);
//	p.x/=consoleCurrentFont.dwFontSize.X;
//	p.y/=consoleCurrentFont.dwFontSize.Y;
//	return p;
//}
//void gotoxy(int x, int y){
//	HANDLE hout;
//	COORD pos;
//	pos.X = x;
//	pos.Y = y;
//	hout = GetStdHandle(STD_OUTPUT_HANDLE);
//	SetConsoleCursorPosition(hout, pos);
//}
