#include "Discord_WOW.h"

#include <Sql/sch_schema.h>
#include <Sql/sch_source.h>

//checking if string could be a number
bool isStringisNumber(Upp::String stringNumber){
	if (std::isdigit(stringNumber[0]) || (stringNumber.GetCount() > 1 && (stringNumber[0] == '+'))){
        for (int i = 1 ; i < stringNumber.GetCount(); ++i)
            if (!std::isdigit(stringNumber[i]))
                return false;
        return true;
    }
    return false;
}
//Function to allow inheritance of type from a string
Value ResolveType2(String valueToResolve){
    if(valueToResolve.GetCount()> 0 && isStringisNumber(valueToResolve)){
        if(valueToResolve.GetCount() > 9){
            return Value(std::stoi(valueToResolve.ToStd()));
        }else if(valueToResolve.Find(",") || valueToResolve.Find(".")){
            return Value(std::stoi(valueToResolve.ToStd()));
        }else{
            return Value(std::stoi(valueToResolve.ToStd()));
        }
    }else if(valueToResolve.GetCount()> 0 && ((valueToResolve[0] == 'b' && isStringisNumber(valueToResolve.Right(valueToResolve.GetCount()-1))) || (ToLower(valueToResolve).IsEqual("true") || ToLower(valueToResolve).IsEqual("false")))  ){
        if(valueToResolve.Find("b")>-1 && isStringisNumber(valueToResolve.Right(valueToResolve.GetCount()-1)) ){
            valueToResolve.Replace("b","");
            return Value(((std::stoi(valueToResolve.ToStd())!=0)? true:false));
        }else if(valueToResolve.IsEqual("true") || valueToResolve.IsEqual("false")){
            return Value(((valueToResolve.IsEqual("true"))? true:false));
        }
    }
    return Value(valueToResolve);

}	

void Discord_WOW::Help(ValueMap payload){
		Upp::String message;
	
	message = "```\n";
	message << "!bl Add(Nom du joueur, motif de l'ajout) -> Ajoute un toxic à la black liste. (Réservée aux membres de la guilde)\n\n";
	message << "!bl Delete(Nom du joueur) -> Supprime un joueur de la liste.(Réservée aux admins)\n\n";
	message << "!bl Check(Nom du joueur) -> Vérifie si un joueur est toxic.\n\n";
	message << "!bl CheckD(Nom du joueu) -> Vérifie si un joueur est toxic et renvois ses motifs si il ait.\n\n";
	message << "!bl credit()" <<" -> Affiche les crédit du module wow blacklist.\n\n";
	message = message << "```";
	
	BotPtr->CreateMessage(this->ChannelLastMessage, message);
}
String Discord_WOW::Credit(ValueMap json,bool sendCredit){
	String credit =  "----World of Warcraft SmartUppBot Module have been made By Clément Hamon---\n";
	credit << "-----------https://github.com/Xemuth-----------\n";
	credit << "Wow module is used to save all scumbag over a database and check for a player if he is clear before inviting him.\n";
	credit << "https://github.com/Xemuth/Discord_WOW";
	if(sendCredit) BotPtr->CreateMessage(ChannelLastMessage,"```"+credit +"```");
	return credit;
}

Discord_WOW::Discord_WOW(Upp::String _name,Upp::String _prefix){
	name = _name;
	AddPrefix(_prefix);
	
	prepareOrLoadBDD();
	LoadMemoryCrud();
	
	EventsMapMessageCreated.Add([&](ValueMap e){if(NameOfFunction.IsEqual("add"))AddPlayer(e);});
	EventsMapMessageCreated.Add([&](ValueMap e){if(NameOfFunction.IsEqual("delete"))RemovePlayer(e);});
	EventsMapMessageCreated.Add([&](ValueMap e){if(NameOfFunction.IsEqual("check"))CheckPlayer(e);});
	EventsMapMessageCreated.Add([&](ValueMap e){if(NameOfFunction.IsEqual("checkd"))DetailledCheckPlayer(e);});
}
void Discord_WOW::EventsMessageCreated(ValueMap payload){
	for(auto &e : EventsMapMessageCreated){
		e(payload);
	}	
}

bool Discord_WOW::CheckRole(ValueMap checkRole,Vector<String>& roleToCheck){
	for(const Value& vm :  checkRole["d"]["member"]["roles"]){
		for(String& role : roleToCheck){
			if(vm.ToString().IsEqual(role)){
				return true;	
			}
		}
	}
	return false;
}

void Discord_WOW::AddPlayer(ValueMap payload){
	if(CheckRole(payload, AllRoleAllowed)){
		if(MessageArgs.GetCount() == 2){
			Value v1 =ResolveType2(MessageArgs[0]);
			Value v2 =ResolveType2(MessageArgs[1]);
			if(v1.GetTypeName().IsEqual("String") && v2.GetTypeName().IsEqual("String")){
				Sql sql(sqlite3);
				String name = ToLower(v1.ToString());
				for( WowPlayer& p : AllWowPlayers){
					if(p.GetPlayerName().IsEqual(name)){
						if(sql*Insert(WOW_MOTIFS)(WOW_MOTIF,v2.ToString())(WOW_MOTIFS_WOW_PLAYER_RECORDS_ID,p.GetPlayerID())){
							p.AddMotifs(v2.ToString());
							BotPtr->CreateMessage(ChannelLastMessage,"Ajout de motif à ce cancer de joueur réussie");
							return;
						}else{
							BotPtr->CreateMessage(ChannelLastMessage,"Erreur d'enregistrement");
							return;						
						}
					}
				}
				if(sql*Insert(WOW_PLAYER_RECORDS)(WOW_NAME,name)){
					WowPlayer& pl = AllWowPlayers.Create(sql.GetInsertedId(),name);
					if(sql*Insert(WOW_MOTIFS)(WOW_MOTIF,v2.ToString())(WOW_MOTIFS_WOW_PLAYER_RECORDS_ID,pl.GetPlayerID())){
						pl.AddMotifs(v2.ToString());
						BotPtr->CreateMessage(ChannelLastMessage,"Enregistrement réussie");
						return;
					}else{
						AllWowPlayers.Remove(AllWowPlayers.GetCount()-1,1);
						BotPtr->CreateMessage(ChannelLastMessage,"Erreur d'enregistrement");
						return;
					}
				}
				BotPtr->CreateMessage(ChannelLastMessage,"Error Registering");
				return;
			}
		}
	}else{
		BotPtr->CreateMessage(ChannelLastMessage,"Vous n'avez pas les droits !");
	}
}

void Discord_WOW::RemovePlayer(ValueMap payload){
	if(CheckRole(payload, AllRoleAdmin)){
		if(MessageArgs.GetCount() == 1){
			Value v1 =ResolveType2(MessageArgs[0]);
			if(v1.GetTypeName().IsEqual("String")){
				Sql sql(sqlite3);
				String name = ToLower(v1.ToString());
				int cpt = 0;
				for( WowPlayer& p : AllWowPlayers){
					if(p.GetPlayerName().IsEqual(name)){
						if(sql*Delete(WOW_PLAYER_RECORDS).Where(WOW_PLAYER_ID == p.GetPlayerID())){
							AllWowPlayers.Remove(cpt,1);
							BotPtr->CreateMessage(ChannelLastMessage,"supression reussite");
							return;
						}else{
							BotPtr->CreateMessage(ChannelLastMessage,"Supression impossible.");
							return;						
						}
					}
					cpt++;
				}
			}else{
				BotPtr->CreateMessage(ChannelLastMessage,"Erreur d'arguments");
			}
		}
	}else{
		BotPtr->CreateMessage(ChannelLastMessage,"Vous n'avez pas les droits !");
	}
}

void Discord_WOW::CheckPlayer(ValueMap payload){
	if(MessageArgs.GetCount() == 1){
		Value v1 =ResolveType2(MessageArgs[0]);			
		if(v1.GetTypeName().IsEqual("String")){
			String name = ToLower(v1.ToString());
			for(WowPlayer& pl : AllWowPlayers){
				if(pl.GetPlayerName().IsEqual(name)){
					BotPtr->CreateMessage(ChannelLastMessage,"Le joueur " + pl.GetPlayerName() + " c'est fait report " + AsString( pl.GetMotifs().GetCount()) +" fois" );
					return;		
				}
			}
			BotPtr->CreateMessage(ChannelLastMessage,"Ce joueur n'est pas un cancer" );
			return;
		}else{
			BotPtr->CreateMessage(ChannelLastMessage,"Erreurs d'arguments" );	
		}
	}else{
		BotPtr->CreateMessage(ChannelLastMessage,"Erreurs d'arguments" );	
	}
}
void Discord_WOW::DetailledCheckPlayer(ValueMap payload){
		if(MessageArgs.GetCount() == 1){
		Value v1 =ResolveType2(MessageArgs[0]);			
		if(v1.GetTypeName().IsEqual("String")){
			String name = ToLower(v1.ToString());
			for(WowPlayer& pl : AllWowPlayers){
				if(pl.GetPlayerName().IsEqual(name)){
					String motif="";
					for(String& s : pl.GetMotifs()){
						motif << s << "\n";	
					}
					BotPtr->CreateMessage(ChannelLastMessage,"Le joueur " + pl.GetPlayerName() + " c'est fait report " + AsString( pl.GetMotifs().GetCount()) +" fois. Voici les motifs : \n" +motif );
					return;		
				}
			}
			BotPtr->CreateMessage(ChannelLastMessage,"Ce joueur n'est pas un cancer" );
			return;
		}else{
			BotPtr->CreateMessage(ChannelLastMessage,"Erreurs d'arguments" );	
		}
	}else{
		BotPtr->CreateMessage(ChannelLastMessage,"Erreurs d'arguments" );	
	}
}



void Discord_WOW::prepareOrLoadBDD(){
	bddLoaded =false;
	sqlite3.LogErrors(true);
	bool mustCreate = false;
	if(!FileExists("Discord_WOW_DataBase.db"))mustCreate =true;
	if(sqlite3.Open("Discord_WOW_DataBase.db")) {
	//	SQL = sqlite3;
		if(mustCreate){
			SqlSchema sch_GB(SQLITE3);
			All_Tables(sch_GB);
			
			if(sch_GB.ScriptChanged(SqlSchema::UPGRADE)){
				SqlPerformScript(sqlite3, sch_GB.Upgrade());
			}	
			if(sch_GB.ScriptChanged(SqlSchema::ATTRIBUTES)){	
				SqlPerformScript(sqlite3,sch_GB.Attributes());
			}
			if(sch_GB.ScriptChanged(SqlSchema::CONFIG)) {
				SqlPerformScript(sqlite3,sch_GB.ConfigDrop());
				SqlPerformScript(sqlite3,sch_GB.Config());
			}
			sch_GB.SaveNormal();
		}
		Sql sql(sqlite3);
		sql.Execute("PRAGMA foreign_keys = ON;"); //Enable Foreign keys
		bddLoaded =true;
	}
	#undef MODEL
}

void Discord_WOW::LoadMemoryCrud(){
	if(bddLoaded){
		AllWowPlayers.Clear();
		
		Sql sql(sqlite3);
		sql*Select(SqlAll()).From(WOW_PLAYER_RECORDS);
		while(sql.Fetch()){
			AllWowPlayers.Create(sql[0].Get<int64>(),sql[1].ToString());	
		}
		for(WowPlayer &p : AllWowPlayers){
			sql*Select(SqlAll()).From(WOW_MOTIFS).Where(WOW_MOTIFS_ID == p.GetPlayerID());
			while(sql.Fetch()){
				p.AddMotifs(sql[1].ToString());
			}
		}
		Cout()<< "Memory crude loaded"<<"\n";//PrintMemoryCrude()<<"\n";
	}
	
}
