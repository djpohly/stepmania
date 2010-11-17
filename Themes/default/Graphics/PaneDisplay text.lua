local iPN = ...;
assert(iPN,"[Graphics/PaneDisplay text.lua] No PlayerNumber Provided.");

local t = Def.ActorFrame {};
local function GetRadarData( pnPlayer, rcRadarCategory )
	local tRadarValues;
	local StepsOrTrail;
	local fDesiredValue = 0;
	if GAMESTATE:GetCurrentSteps( pnPlayer ) then
		StepsOrTrail = GAMESTATE:GetCurrentSteps( pnPlayer );
		fDesiredValue = StepsOrTrail:GetRadarValues( pnPlayer ):GetValue( rcRadarCategory );
	elseif GAMESTATE:GetCurrentTrail( pnPlayer ) then
		StepsOrTrail = GAMESTATE:GetCurrentTrail( pnPlayer );
		fDesiredValue = StepsOrTrail:GetRadarValues( pnPlayer ):GetValue( rcRadarCategory );
	else
		StepsOrTrail = nil;
	end;
	return fDesiredValue;
end;

local function CreatePaneDisplayItem( _pnPlayer, _sLabel, _rcRadarCategory )
	return Def.ActorFrame {
		LoadFont("Common Normal") .. {
			Text=_sLabel;
			InitCommand=cmd(horizalign,left);
			OnCommand=cmd(zoom,0.5;diffuse,color("0.9,0.9,0.9");shadowlength,1);
		};
		LoadFont("Common Normal") .. {
			Text="000";
			InitCommand=cmd(x,32+8+4;horizalign,left);
			OnCommand=cmd(zoom,0.5;shadowlength,1);
			CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
			CurrentStepsP1ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentStepsP2ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentTrailP1ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentTrailP2ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
			SetCommand=function(self)
				self:settextf("%03i", GetRadarData( _pnPlayer, _rcRadarCategory ) );
			end;
		};
	};
end;

local function CreatePaneDisplayGraph( _pnPlayer, _sLabel, _rcRadarCategory )
	return Def.ActorFrame {
		LoadFont("Common Normal") .. {
			Text=_sLabel;
			InitCommand=cmd(horizalign,left);
			OnCommand=cmd(zoom,0.5;shadowlength,1);
		};
		Def.Quad { 
			InitCommand=cmd(x,12;zoomto,50,10;horizalign,left);
			OnCommand=cmd(diffuse,Color("Black");shadowlength,1;diffusealpha,0.5);
		};
		Def.Quad {
			InitCommand=cmd(x,12;zoomto,50,10;horizalign,left);
			OnCommand=cmd(shadowlength,0;diffuse,Color("Green");diffusebottomedge,ColorLightTone(Color("Green")));
			CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
			CurrentStepsP1ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentStepsP2ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentTrailP1ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentTrailP2ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
			SetCommand=function(self)
				self:zoomtowidth( GetRadarData( _pnPlayer, _rcRadarCategory ) * 50 );
			end;
		};
		LoadFont("Common Normal") .. {
			InitCommand=cmd(x,40;zoom,0.5;);
			OnCommand=cmd(shadowlength,1;);
			CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
			CurrentStepsP1ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentStepsP2ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentTrailP1ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentTrailP2ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
			SetCommand=function(self)
				self:settextf("%i%%", GetRadarData( _pnPlayer, _rcRadarCategory ) * 100 );
			end;
		};
	};
end;

--[[ Numbers ]]
t[#t+1] = Def.ActorFrame {
	-- Left 
	CreatePaneDisplayItem( iPN, "TAPS", 'RadarCategory_TapsAndHolds' ) .. {
		InitCommand=cmd(x,-128+16+8;y,-14);
	};
	CreatePaneDisplayItem( iPN, "JUMPS", 'RadarCategory_Jumps' ) .. {
		InitCommand=cmd(x,-128+16+8;y,-14+16);
	};
	CreatePaneDisplayItem( iPN, "HOLDS", 'RadarCategory_Holds' ) .. {
		InitCommand=cmd(x,-128+16+8;y,-14+16*2);
	};
	CreatePaneDisplayItem( iPN, "MINES", 'RadarCategory_Mines' ) .. {
		InitCommand=cmd(x,-128+16+8;y,-14+16*3);
	};
	-- Center
	CreatePaneDisplayItem( iPN, "MINES", 'RadarCategory_Mines' ) .. {
		InitCommand=cmd(x,-128+16+8+74;y,-14);
	};
	CreatePaneDisplayItem( iPN, "HANDS", 'RadarCategory_Hands' ) .. {
		InitCommand=cmd(x,-128+16+8+74;y,-14+16);
	};
	CreatePaneDisplayItem( iPN, "ROLLS", 'RadarCategory_Rolls' ) .. {
		InitCommand=cmd(x,-128+16+8+74;y,-14+16*2);
	};
	CreatePaneDisplayItem( iPN, "LIFTS", 'RadarCategory_Lifts' ) .. {
		InitCommand=cmd(x,-128+16+8+74;y,-14+16*3);
	};
	-- Right
	CreatePaneDisplayGraph( iPN, "S", 'RadarCategory_Stream' ) .. {
		InitCommand=cmd(x,-128+16+8+74*2;y,-14);
	};
	CreatePaneDisplayGraph( iPN, "V", 'RadarCategory_Voltage' ) .. {
		InitCommand=cmd(x,-128+16+8+74*2;y,-14+12);
	};
	CreatePaneDisplayGraph( iPN, "A", 'RadarCategory_Air' ) .. {
		InitCommand=cmd(x,-128+16+8+74*2;y,-14+12*2);
	};
	CreatePaneDisplayGraph( iPN, "F", 'RadarCategory_Freeze' ) .. {
		InitCommand=cmd(x,-128+16+8+74*2;y,-14+12*3);
	};
	CreatePaneDisplayGraph( iPN, "C", 'RadarCategory_Chaos' ) .. {
		InitCommand=cmd(x,-128+16+8+74*2;y,-14+12*4);
	};
};
return t;