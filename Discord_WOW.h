#ifndef _Discord_WOW_Discord_WOW_h_
#define _Discord_WOW_Discord_WOW_h_
#include <SmartUppBot/SmartBotUpp.h>
#include "WOW_MemoryCRUD.h"
#define NOAPPSQL
#include <plugin/sqlite3/Sqlite3.h>
using namespace Upp;

#define MODEL <Discord_WOW/Discord_WOW.sch>
#define SCHEMADIALECT  <plugin/sqlite3/Sqlite3Schema.h>
#include "Sql/sch_header.h"

class Discord_WOW: public DiscordModule{
	private:
		void PrepareEvent();
		//Commands
		void AddPlayer(ValueMap& payload);
		void CheckPlayer(ValueMap& payload);
		void DetailledCheckPlayer(ValueMap& payload);
		void RemovePlayer(ValueMap& payload);
		void Help(ValueMap& payload);
		virtual String Credit(ValueMap& payload,bool sendCredit = true);
		//**************
		
		void prepareOrLoadBDD();
		void LoadMemoryCrud();
		
		Vector<WowPlayer> AllWowPlayers;
		Sqlite3Session sqlite3; //DataBase
		bool bddLoaded =false;

		bool CheckRole(ValueMap checkRole,Vector<String>& roleToCheck);
		Vector<String> AllRoleAllowed{
			"627434603560304700",
			"415115145459662849"
		};
		Vector<String> AllRoleAdmin{
			"627434899573309490",
			"597876891827044382"
		};
	
	public:
		
	
		
		Discord_WOW(Upp::String _name,Upp::String _prefix);
		void EventsMessageCreated(ValueMap payload);
};
#endif
