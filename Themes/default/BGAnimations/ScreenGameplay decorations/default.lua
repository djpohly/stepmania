local function CreateStops()
	local t = Def.ActorFrame { };
	local bars = Def.ActorFrame{ };
	local bpmFrame = Def.ActorFrame{ Name="BPMFrame"; };
	local stopFrame = Def.ActorFrame{ Name="StopFrame"; };

	local fFrameWidth = 380;
	local fFrameHeight = 8;
	-- XXX: doesn't work in course mode -aj
	if not GAMESTATE:IsCourseMode() then
	-- Straight rip off NCRX
		local song = GAMESTATE:GetCurrentSong();
		local timingData = song:GetTimingData();
		-- if we're using SSC, might as well use the StepsSeconds, which will
		-- almost always be more proper than a r21'd file.
		if song then
			local songLen = song:MusicLengthSeconds();

			local firstBeatSecs = timingData:GetElapsedTimeFromBeat(song:GetFirstBeat());
			local lastBeatSecs = timingData:GetElapsedTimeFromBeat(song:GetLastBeat());

			local bpms = timingData:GetBPMs();

			local stops = timingData:GetStops();
			for i=1,#stops do
				local data = split("=",stops[i]);
				local beat = data[1];
				local secs = data[2];
				local beatTime = timingData:GetElapsedTimeFromBeat(beat);
				if beatTime < 0 then beatTime = 0; end;

				stopFrame[#stopFrame+1] = Def.ActorFrame {
					Def.Quad {
						InitCommand=function(self)
							--self:diffuse(HSVA(192,1,0.8,0.8));
							self:shadowlength(0);
							self:shadowcolor( color("#FFFFFF77") );
							-- set width
							self:zoomto( math.max((secs/songLen)*fFrameWidth, 1), fFrameHeight );
							-- find location
							self:x( ( scale(beatTime, firstBeatSecs,lastBeatSecs, -fFrameWidth/2,fFrameWidth/2) ) );
						end;
						OnCommand=function(self)
							self:diffuse(Color("White"));
							self:sleep(beatTime+1);
							self:linear(2);
							self:diffusealpha(0);
						end;
					};
					Def.Quad {
						InitCommand=function(self)
							--self:diffuse(HSVA(192,1,0.8,0.8));
							self:shadowlength(0);
							self:shadowcolor( color("#FFFFFF77") );
							-- set width
							self:zoomto( math.max((secs/songLen)*fFrameWidth, 1), fFrameHeight );
							-- find location
							self:x( ( scale(beatTime, firstBeatSecs,lastBeatSecs, -fFrameWidth/2,fFrameWidth/2) ) );
						end;
						OnCommand=function(self)
							self:diffusealpha(1);
							self:diffuseshift();
							self:effectcolor1(Color("Orange"));
							self:effectcolor2(Color("Red"));
							self:effectclock('beat');
							self:effectperiod(1/8);
							--
							self:diffusealpha(0);
							self:sleep(beatTime+1);
	-- 						self:linear(1);
							self:diffusealpha(1);
							self:linear(4);
							self:diffusealpha(0);
						end;
					};
					--OnCommand=cmd(diffuse,Color("White");sleep,math.min(0.00001+(secs-5),0.00001);linear,1;diffuse,Color("Orange");sleep,4;linear,2;diffusealpha,0);
					--OnCommand=cmd(diffuse,Color("White");linear,1;diffuse,Color("Orange"););
				};
			end;
		end;
		bars[#bars+1] = stopFrame;
		t[#t+1] = bars;
	end
	return t
end
local t = LoadFallbackB()
for pn in ivalues(PlayerNumber) do
-- 	local MetricsName = "SongMeterDisplay" .. PlayerNumberToString(pn);
--[[ 	t[#t+1] = Def.ActorFrame {
		InitCommand=function(self) 
			self:player(pn); 
			self:name(MetricsName); 
			ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen"); 
		end;
 		LoadActor( THEME:GetPathG( 'SongMeterDisplay', 'frame ' .. PlayerNumberToString(pn) ) ) .. {
			InitCommand=function(self)
				self:name('Frame'); 
				ActorUtil.LoadAllCommandsAndSetXY(self,MetricsName); 
			end;
		};
		Def.Quad {
			InitCommand=cmd(zoomto,2,8);
			OnCommand=cmd(x,scale(0.25,0,1,-380/2,380/2);diffuse,PlayerColor(pn);diffusealpha,0.5);
		};
		Def.Quad {
			InitCommand=cmd(zoomto,2,8);
			OnCommand=cmd(x,scale(0.5,0,1,-380/2,380/2);diffuse,PlayerColor(pn);diffusealpha,0.5);
		};
		Def.Quad {
			InitCommand=cmd(zoomto,2,8);
			OnCommand=cmd(x,scale(0.75,0,1,-380/2,380/2);diffuse,PlayerColor(pn);diffusealpha,0.5);
		};
		Def.SongMeterDisplay {
			StreamWidth=THEME:GetMetric( MetricsName, 'StreamWidth' );
			Stream=LoadActor( THEME:GetPathG( 'SongMeterDisplay', 'stream ' .. PlayerNumberToString(pn) ) )..{
				InitCommand=cmd(diffuse,PlayerColor(pn);diffusealpha,0.5;blend,Blend.Add;);
			};
			Tip=LoadActor( THEME:GetPathG( 'SongMeterDisplay', 'tip ' .. PlayerNumberToString(pn) ) ) .. { InitCommand=cmd(visible,false); };
		};
		CreateStops();
	}; --]]
end;
for pn in ivalues(PlayerNumber) do
	local MetricsName = "ToastyDisplay" .. PlayerNumberToString(pn);
	t[#t+1] = LoadActor( THEME:GetPathG("Player", 'toasty'), pn ) .. {
		InitCommand=function(self) 
			self:player(pn); 
			self:name(MetricsName); 
			ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen"); 
		end;
	};
end;
t[#t+1] = StandardDecorationFromFileOptional("BPMDisplay","BPMDisplay");
t[#t+1] = StandardDecorationFromFileOptional("StageDisplay","StageDisplay");
t[#t+1] = StandardDecorationFromFileOptional("SongTitle","SongTitle");
t[#t+1] = Def.ActorFrame {
	InitCommand=function(self)
		self:name("SongMeterDisplay"); 
		ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen"); 
	end;
	LoadActor( THEME:GetPathG( 'SongMeterDisplay', 'frame ' .. PlayerNumberToString(PLAYER_1) ) ) .. {
		InitCommand=function(self)
			self:name('Frame'); 
			ActorUtil.LoadAllCommandsAndSetXY(self,"SongMeterDisplay"); 
		end;
	};
	Def.Quad {
		InitCommand=cmd(zoomto,2,8);
		OnCommand=cmd(x,scale(0.25,0,1,-380/2,380/2);diffuse,Color("Orange");diffusealpha,0.5);
	};
	Def.Quad {
		InitCommand=cmd(zoomto,2,8);
		OnCommand=cmd(x,scale(0.5,0,1,-380/2,380/2);diffuse,Color("Orange");diffusealpha,0.5);
	};
	Def.Quad {
		InitCommand=cmd(zoomto,2,8);
		OnCommand=cmd(x,scale(0.75,0,1,-380/2,380/2);diffuse,Color("Orange");diffusealpha,0.5);
	};
	Def.SongMeterDisplay {
		StreamWidth=THEME:GetMetric( "SongMeterDisplay", 'StreamWidth' );
		Stream=LoadActor( THEME:GetPathG( 'SongMeterDisplay', 'stream ' .. PlayerNumberToString(PLAYER_1) ) )..{
			InitCommand=cmd(diffuse,Color("Orange");diffusealpha,0.5;blend,Blend.Add;);
		};
		Tip=LoadActor( THEME:GetPathG( 'SongMeterDisplay', 'tip ' .. PlayerNumberToString(PLAYER_1) ) ) .. { InitCommand=cmd(visible,false); };
	};
	CreateStops();
};
return t