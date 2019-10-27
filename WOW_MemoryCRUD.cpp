#include "WOW_MemoryCRUD.h"
		
WowPlayer::WowPlayer(int id,String name){
	playerID=id;
	playerName=name;
}
String& WowPlayer::AddMotifs(String _motif){
	return motifs.Add(_motif);
}
Vector<String>& WowPlayer::GetMotifs(){
	return motifs;
}

int WowPlayer::GetPlayerID(){
	return playerID;
}
String WowPlayer::GetPlayerName(){
	return playerName;
}
