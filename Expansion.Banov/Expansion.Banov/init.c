/**
 * init.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2020 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

#include "$CurrentDir:\\mpmissions\\Expansion.Banov\\expansion\\ExpansionObjectSpawnTools.c"
#include "$CurrentDir:\\mpmissions\\Expansion.Banov\\expansion\\missions\\MissionConstructor.c"

static void SpawnObject(string type, vector position, vector orientation)
{
    auto obj = GetGame().CreateObjectEx(type, position, ECE_SETUP | ECE_UPDATEPATHGRAPH | ECE_CREATEPHYSICS);
    obj.SetPosition(position);
    obj.SetOrientation(orientation);
    obj.SetOrientation(obj.GetOrientation());
    obj.SetFlags(EntityFlags.STATIC, false);
    obj.Update();
	obj.SetAffectPathgraph(true, false);
	if (obj.CanAffectPathgraph()) GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(GetGame().UpdatePathgraphRegionByObject, 100, false, obj);
}

void main()
{
	bool loadTraderObjects = false;
	bool loadTraderNPCs = false;

	string MissionWorldName = "empty";
	GetGame().GetWorldName(MissionWorldName);

	if (MissionWorldName != "empty")
	{
		//! Spawn mission objects and traders
		FindMissionFiles(MissionWorldName, loadTraderObjects, loadTraderNPCs);
	}
	

	//INIT WEATHER BEFORE ECONOMY INIT  newest copy   ------------------------
    Weather weather = g_Game.GetWeather();
    weather.MissionWeather(false);    // false = use weather controller from Weather.c

    weather.GetOvercast().SetLimits( 0.25 , 0.75 );//
    weather.GetRain().SetLimits(0.0, 0.4);//
    weather.GetFog().SetLimits(0.1, 0.28);

    weather.GetOvercast().SetForecastChangeLimits(1.0, 1.0);
    weather.GetRain().SetForecastChangeLimits(0.0, 0.05);//
    weather.GetFog().SetForecastChangeLimits(0.10, 0.25);//

    weather.GetOvercast().SetForecastTimeLimits(1800, 1800);
    weather.GetRain().SetForecastTimeLimits(150, 500);//
    weather.GetFog().SetForecastTimeLimits(600, 600);//

    weather.GetOvercast().Set(Math.RandomFloatInclusive(0.3, 0.4), 0, 0);
    weather.GetRain().Set(Math.RandomFloatInclusive(0.0, 0.2), 0, 0);
    weather.GetFog().Set(Math.RandomFloatInclusive(0, 1), 0, 0);

    weather.SetWindMaximumSpeed(15);//
    weather.SetWindFunctionParams(0.1, 1.0, 50);

	//INIT ECONOMY--------------------------------------
	Hive ce = CreateHive();
	if ( ce )
		ce.InitOffline();
	
	//custom map file goes here - dont forget to turn off tunnel if creating loot mapgrouppos file
	//Dis_NEAF_Quarantine_Z();
	//Dis_Oilrig();
	//DisTunnel();
	
	
	//open this line to creat mapgroupos file - can be found in storage_xx export folder
	//GetCEApi().ExportProxyData( "7500 0 7500", 10000 );  //Center of map, radius of how far to go out and find buildings.

	//DATE RESET AFTER ECONOMY INIT-------------------------
	int year, month, day, hour, minute;
	int reset_month = 8, reset_day = 10;
	GetGame().GetWorld().GetDate(year, month, day, hour, minute);

	if ((month == reset_month) && (day < reset_day))
	{
		GetGame().GetWorld().SetDate(year, reset_month, reset_day, hour, minute);
	}
	else
	{
		if ((month == reset_month + 1) && (day > reset_day))
		{
			GetGame().GetWorld().SetDate(year, reset_month, reset_day, hour, minute);
		}
		else
		{
			if ((month < reset_month) || (month > reset_month + 1))
			{
				GetGame().GetWorld().SetDate(year, reset_month, reset_day, hour, minute);
			}
		}
	}
}

class CustomMission: MissionServer
{	
	// ------------------------------------------------------------
	// Override OnInit
	// ------------------------------------------------------------
	override void OnInit()
	{
		ExpansionMissionModule missionModule;
		if ( Class.CastTo( missionModule, GetModuleManager().GetModule( ExpansionMissionModule ) ) )
		{
			missionModule.SetMissionConstructor( COMMissionConstructor );
		}

		super.OnInit();
	}
	
	// ------------------------------------------------------------
	// Override CreateCharacter
	// ------------------------------------------------------------
	override PlayerBase CreateCharacter(PlayerIdentity identity, vector pos, ParamsReadContext ctx, string characterName)
	{
		Entity playerEnt;
		playerEnt = GetGame().CreatePlayer( identity, characterName, pos, 0, "NONE" );
		Class.CastTo( m_player, playerEnt );

		GetGame().SelectPlayer( identity, m_player );

		return m_player;
	}
	
	// ------------------------------------------------------------
	// SetRandomHealth
	// ------------------------------------------------------------
	void SetRandomHealth(EntityAI itemEnt)
	{
		if ( itemEnt )
		{
			float rndHlt = Math.RandomFloat( 0.25, 0.65 );
			itemEnt.SetHealth01( "", "", rndHlt );
		}
	}
	
	// ------------------------------------------------------------
	// StartingEquipSetup
	// ------------------------------------------------------------
	override void StartingEquipSetup(PlayerBase player, bool clothesChosen)
	{
		if ( !GetExpansionSettings().GetSpawn().StartingClothing.EnableCustomClothing )
		{
			EntityAI itemClothing;
			EntityAI itemEnt;
			ItemBase itemBs;
			float rand;

			itemClothing = player.FindAttachmentBySlotName( "Body" );
			if ( itemClothing )
			{
				SetRandomHealth( itemClothing );
				
				itemEnt = itemClothing.GetInventory().CreateInInventory( "Rag" );
				if ( Class.CastTo( itemBs, itemEnt ) )
					itemBs.SetQuantity( 4 );

				SetRandomHealth( itemEnt );

				string chemlightArray[] = { "Chemlight_White", "Chemlight_Yellow", "Chemlight_Green", "Chemlight_Red" };
				int rndIndex = Math.RandomInt( 0, 4 );
				itemEnt = itemClothing.GetInventory().CreateInInventory( chemlightArray[rndIndex] );
				SetRandomHealth( itemEnt );

				rand = Math.RandomFloatInclusive( 0.0, 1.0 );
				if ( rand < 0.35 )
					itemEnt = player.GetInventory().CreateInInventory( "Apple" );
				else if ( rand > 0.65 )
					itemEnt = player.GetInventory().CreateInInventory( "Pear" );
				else
					itemEnt = player.GetInventory().CreateInInventory( "Plum" );

				SetRandomHealth( itemEnt );
			}
			
			itemClothing = player.FindAttachmentBySlotName( "Legs" );
			if ( itemClothing )
				SetRandomHealth( itemClothing );
			
			itemClothing = player.FindAttachmentBySlotName( "Feet" );
			if ( itemClothing )
				SetRandomHealth( itemClothing );
		}
	}
}

Mission CreateCustomMission(string path)
{
	return new CustomMission();
}