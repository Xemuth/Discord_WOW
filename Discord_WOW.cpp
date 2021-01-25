#include "Discord_WOW.h"

#include <Sql/sch_schema.h>
#include <Sql/sch_source.h>

void Discord_WOW::PrepareEvent(){
	EventsMapMessageCreated.Add([&](ValueMap& e){if(NameOfFunction.IsEqual("add"))AddPlayer(e);});
	EventsMapMessageCreated.Add([&](ValueMap& e){if(NameOfFunction.IsEqual("delete"))RemovePlayer(e);});
	EventsMapMessageCreated.Add([&](ValueMap& e){if(NameOfFunction.IsEqual("check"))CheckPlayer(e);});
	EventsMapMessageCreated.Add([&](ValueMap& e){if(NameOfFunction.IsEqual("checkd"))DetailledCheckPlayer(e);});
	EventsMapMessageCreated.Add([&](ValueMap& e){if(NameOfFunction.IsEqual("hello"))HelloWorld(e);});
}
	
void Discord_WOW::Help(ValueMap& payload){
	Upp::String message;
	
	message = "```\n";
	message << "*******'Ops' veut dire Optionnel. Chaque Argument noté Optionnel n'est pas obligatoire.*******\n\n";
	message << "!bl Add(Name:Xemuth; Ops->Motif:Est manifestement beaucoup trop fort) -> Ajoute un toxic à la black liste(Réservée aux membres de la guilde). Les Arguments sont 'Name' et 'Motif'\n\n";
	message << "!bl Delete(Name:Xanf) -> Supprime un joueur de la liste.(Réservée aux admins). Un seul Argument 'Name'\n\n";
	message << "!bl Check(Name:Arthons) -> Vérifie si un joueur est toxic.\n\n";
	message << "!bl CheckD(Name:Rugissang) -> Vérifie si un joueur est toxic et renvois ses motifs si il ait.\n\n";
	message << "!bl Liste -> Affiche la liste de tous les joueurs bannis.(Pas encore active)\n\n";
	message << "!bl Credit" <<" -> Affiche les crédit du module wow blacklist.\n\n";
	message = message << "```";
	
	BotPtr->CreateMessage(this->ChannelLastMessage, message);
}
String Discord_WOW::Credit(ValueMap& json,bool sendCredit){
	String credit =  "----World of Warcraft SmartUppBot Module have been made By Clément Hamon---\n";
	credit << "-----------https://github.com/Xemuth-----------\n";
	credit << "Wow module is used to save all scumbag over a database and check for a player if he is clear before inviting him.\n";
	credit << "https://github.com/Xemuth/Discord_WOW";
	if(sendCredit) BotPtr->CreateMessage(ChannelLastMessage,"```"+credit +"```");
	return credit;
}

void Discord_WOW::HelloWorld(ValueMap& payload){
	String hello =  "Wow ! ça faisait longtemps !";
	BotPtr->CreateMessage(ChannelLastMessage,hello);
}

Discord_WOW::Discord_WOW(Upp::String _name,const Vector<String>& _prefix){
	name = _name;
	AddPrefix(_prefix);
	
	prepareOrLoadBDD();
	LoadMemoryCrud();
	PrepareEvent();
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

void Discord_WOW::AddPlayer(ValueMap& payload){
	if(CheckRole(payload, AllRoleAllowed)){
		String name="";
		String motif="";
		if(MessageArgs.Find("name") != -1 &&  MessageArgs.Get("name").GetTypeName().IsEqual("String")) name = MessageArgs.Get("name").ToString();
		if(MessageArgs.Find("motif") != -1 &&  MessageArgs.Get("motif").GetTypeName().IsEqual("String")) motif = MessageArgs.Get("motif").ToString();
		if(motif.GetCount() ==0) motif ="Aucun Motif défini";
		if(name.GetCount() > 0){
			Sql sql(sqlite3);
			name = ToLower(name);
			for( WowPlayer& p : AllWowPlayers){
				if(p.GetPlayerName().IsEqual(name)){
					if(sql*Insert(WOW_MOTIFS)(WOW_MOTIF,motif)(WOW_MOTIFS_WOW_PLAYER_RECORDS_ID,p.GetPlayerID())){
						p.AddMotifs(motif);
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
				if(sql*Insert(WOW_MOTIFS)(WOW_MOTIF,motif)(WOW_MOTIFS_WOW_PLAYER_RECORDS_ID,pl.GetPlayerID())){
					pl.AddMotifs(motif);
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
		}else{
			BotPtr->CreateMessage(ChannelLastMessage,"Spécifie moi un nom ! Comme ça : !bl Add(Name: <@!" + AuthorId +">; Motif: A pas compris la commande discord) Tu es pas obligé de mettre un motif");
		}
	}else{
		BotPtr->CreateMessage(ChannelLastMessage,"Vous n'avez pas les droits !");
	}
}

void Discord_WOW::RemovePlayer(ValueMap& payload){
	if(CheckRole(payload, AllRoleAdmin)){
		String name="";
		if(MessageArgs.Find("name") != -1 &&  MessageArgs.Get("name").GetTypeName().IsEqual("String")) name = MessageArgs.Get("name").ToString();
		if(name.GetCount() >0){
			Sql sql(sqlite3);
			name = ToLower(name);
			int cpt = 0;
			for( WowPlayer& p : AllWowPlayers){
				if(p.GetPlayerName().IsEqual(name)){
					if(sql*Delete(WOW_PLAYER_RECORDS).Where(WOW_PLAYER_ID == p.GetPlayerID())){
						AllWowPlayers.Remove(cpt,1);
						BotPtr->CreateMessage(ChannelLastMessage,"Supression reussite !");
						return;
					}else{
						BotPtr->CreateMessage(ChannelLastMessage,"Supression impossible !");
						return;						
					}
				}
				cpt++;
			}
			BotPtr->CreateMessage(ChannelLastMessage,"Je n'ai pas trouvé ce joueur !");
		}else{
			BotPtr->CreateMessage(ChannelLastMessage,"Donne moi un nom !  Comme ça  : !bl delete(Name:<@!" + AuthorId +">)");
		}
	}else{
		BotPtr->CreateMessage(ChannelLastMessage,"Heyyyyy mais tu es pas Admin la ! je vais te marabouter si tu continues");
	}
}

void Discord_WOW::CheckPlayer(ValueMap& payload){	
	String name="";
	if(MessageArgs.Find("name") != -1 &&  MessageArgs.Get("name").GetTypeName().IsEqual("String")) name = MessageArgs.Get("name").ToString();
	if(name.GetCount()>0){
		name= ToLower(name);
		for(WowPlayer& pl : AllWowPlayers){
			if(pl.GetPlayerName().IsEqual(name)){
				BotPtr->CreateMessage(ChannelLastMessage,"Le joueur " + pl.GetPlayerName() + " c'est fait report " + AsString( pl.GetMotifs().GetCount()) +" fois" );
				return;		
			}
		}
		BotPtr->CreateMessage(ChannelLastMessage,"Ce joueur n'est pas un cancer" );
		return;
	}else{
		BotPtr->CreateMessage(ChannelLastMessage,"Donne moi un nom !  Comme ça  : !bl Check(Name:<@!" + AuthorId +">)");	
	}
}
void Discord_WOW::DetailledCheckPlayer(ValueMap& payload){
	String name="";
	if(MessageArgs.Find("name") != -1 &&  MessageArgs.Get("name").GetTypeName().IsEqual("String")) name = MessageArgs.Get("name").ToString();
	if(name.GetCount()>0){
		name= ToLower(name);
		for(WowPlayer& pl : AllWowPlayers){
			if(pl.GetPlayerName().IsEqual(name)){
				String motif="";
				for(String& s : pl.GetMotifs()){
					motif << s << "\n";	
				}
				BotPtr->CreateMessage(ChannelLastMessage,"Le joueur " + pl.GetPlayerName() + " c'est fait report " + AsString( pl.GetMotifs().GetCount()) +" fois. Voici les motifs : \n" + motif );
				return;		
			}
		}
		BotPtr->CreateMessage(ChannelLastMessage,"Ce joueur n'est pas un cancer" );
		return;
	}else{
		BotPtr->CreateMessage(ChannelLastMessage,"Donne moi un nom !  Comme ça  : !bl CheckD(Name:<@!" + AuthorId +">)");	
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
