#ifndef _Discord_WOW_WOW_MemoryCRUD_h_
#define _Discord_WOW_WOW_MemoryCRUD_h_

#include <Core/Core.h>

using namespace Upp;

class WowPlayer : public Moveable<WowPlayer>{
	private:
		int playerID;
		String playerName;
		Vector<String> motifs;
			
	public:
		
		int GetPlayerID();
		String GetPlayerName();
		
		WowPlayer(int id,String name);
		 
		String& AddMotifs(String _motif);
		Vector<String>& GetMotifs();
	
};

#endif
