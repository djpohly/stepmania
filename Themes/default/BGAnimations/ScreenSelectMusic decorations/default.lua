local t = LoadFallbackB();

-- Legacy StepMania 4 Function
local function StepsDisplay(pn)
	local function set(self, player)
		self:SetFromGameState( player );
	end

	local t = Def.StepsDisplay {
		InitCommand=cmd(Load,"StepsDisplay",GAMESTATE:GetPlayerState(pn););
	};

	if pn == PLAYER_1 then
		t.CurrentStepsP1ChangedMessageCommand=function(self) set(self, pn); end;
		t.CurrentTrailP1ChangedMessageCommand=function(self) set(self, pn); end;
	else
		t.CurrentStepsP2ChangedMessageCommand=function(self) set(self, pn); end;
		t.CurrentTrailP2ChangedMessageCommand=function(self) set(self, pn); end;
	end

	return t;
end
t[#t+1] = StandardDecorationFromFileOptional("AlternateHelpDisplay","AlternateHelpDisplay");

local function PercentScore(pn)
	local t = LoadFont("Common normal")..{
		InitCommand=cmd(zoom,0.625;shadowlength,1);
		BeginCommand=cmd(playcommand,"Set");
		SetCommand=function(self)
			local SongOrCourse, StepsOrTrail;
			if GAMESTATE:IsCourseMode() then
				SongOrCourse = GAMESTATE:GetCurrentCourse();
				StepsOrTrail = GAMESTATE:GetCurrentTrail(pn);
			else
				SongOrCourse = GAMESTATE:GetCurrentSong();
				StepsOrTrail = GAMESTATE:GetCurrentSteps(pn);
			end;

			local profile, scorelist;
			local text = "";
			if SongOrCourse and StepsOrTrail then
				local st = StepsOrTrail:GetStepsType();
				local diff = StepsOrTrail:GetDifficulty();
				local courseType = GAMESTATE:IsCourseMode() and SongOrCourse:GetCourseType() or nil;
				local cd = GetCustomDifficulty(st, diff, courseType);
				self:diffuse(CustomDifficultyToColor(cd));
				self:shadowcolor(CustomDifficultyToDarkColor(cd));

				if PROFILEMAN:IsPersistentProfile(pn) then
					-- player profile
					profile = PROFILEMAN:GetProfile(pn);
				else
					-- machine profile
					profile = PROFILEMAN:GetMachineProfile();
				end;

				scorelist = profile:GetHighScoreList(SongOrCourse,StepsOrTrail);
				assert(scorelist)
				local scores = scorelist:GetHighScores();
				local topscore = scores[1];
				if topscore then
					text = string.format("%.2f%%", topscore:GetPercentDP()*100.0);
					-- 100% hack
					if text == "100.00%" then
						text = "100%";
					end;
				else
					text = string.format("%.2f%%", 0);
				end;
			else
				text = "";
			end;
			self:settext(text);
		end;
		CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
		CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
	};

	if pn == PLAYER_1 then
		t.CurrentStepsP1ChangedMessageCommand=cmd(playcommand,"Set");
		t.CurrentTrailP1ChangedMessageCommand=cmd(playcommand,"Set");
	else
		t.CurrentStepsP2ChangedMessageCommand=cmd(playcommand,"Set");
		t.CurrentTrailP2ChangedMessageCommand=cmd(playcommand,"Set");
	end

	return t;
end

-- Legacy StepMania 4 Function
for pn in ivalues(PlayerNumber) do
	local MetricsName = "StepsDisplay" .. PlayerNumberToString(pn);
	t[#t+1] = StepsDisplay(pn) .. {
		InitCommand=function(self) self:player(pn); self:name(MetricsName); ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen"); end;
		PlayerJoinedMessageCommand=function(self, params)
			if params.Player == pn then
				self:visible(true);
				(cmd(zoom,0;bounceend,0.3;zoom,1))(self);
			end;
		end;
		PlayerUnjoinedMessageCommand=function(self, params)
			if params.Player == pn then
				self:visible(true);
				(cmd(bouncebegin,0.3;zoom,0))(self);
			end;
		end;
	};
	if ShowStandardDecoration("PercentScore"..ToEnumShortString(pn)) then
		t[#t+1] = StandardDecorationFromTable("PercentScore"..ToEnumShortString(pn), PercentScore(pn));
	end;
end

t[#t+1] = StandardDecorationFromFileOptional("BannerFrame","BannerFrame");
t[#t+1] = StandardDecorationFromFileOptional("PaneDisplayFrameP1","PaneDisplayFrame");
t[#t+1] = StandardDecorationFromFileOptional("PaneDisplayFrameP2","PaneDisplayFrame");
t[#t+1] = StandardDecorationFromFileOptional("PaneDisplayTextP1","PaneDisplayTextP1");
t[#t+1] = StandardDecorationFromFileOptional("PaneDisplayTextP2","PaneDisplayTextP2");
t[#t+1] = StandardDecorationFromFileOptional("DifficultyList","DifficultyList");
t[#t+1] = StandardDecorationFromFileOptional("CourseContentsList","CourseContentsList");
t[#t+1] = StandardDecorationFromFileOptional("BPMDisplay","BPMDisplay");
t[#t+1] = StandardDecorationFromFileOptional("BPMLabel","BPMLabel");
--[[ t[#t+1] = StandardDecorationFromFileOptional("NegativeDisplay","NegativeDisplay") .. {
}; --]]

t[#t+1] = StandardDecorationFromFileOptional("SongTime","SongTime") .. {
	SetCommand=function(self)
		local curSelection = nil;
		local length = 0.0;
		if GAMESTATE:IsCourseMode() then
			curSelection = GAMESTATE:GetCurrentCourse();
			self:playcommand("Reset");
			if curSelection then
				local trail = GAMESTATE:GetCurrentTrail(GAMESTATE:GetMasterPlayerNumber());
				if trail then
					length = TrailUtil.GetTotalSeconds(trail);
				else
					length = 0.0;
				end;
			else
				length = 0.0;
			end;
		else
			curSelection = GAMESTATE:GetCurrentSong();
			self:playcommand("Reset");
			if curSelection then
				length = curSelection:MusicLengthSeconds();
				if curSelection:IsLong() then
					self:playcommand("Long");
				elseif curSelection:IsMarathon() then
					self:playcommand("Marathon");
				else
					self:playcommand("Reset");
				end
			else
				length = 0.0;
				self:playcommand("Reset");
			end;
		end;
		self:settext( SecondsToMSS(length) );
	end;
	CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
	CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
	CurrentTrailP1ChangedMessageCommand=cmd(playcommand,"Set");
	CurrentTrailP2ChangedMessageCommand=cmd(playcommand,"Set");
}

if not GAMESTATE:IsCourseMode() then
	t[#t+1] = StandardDecorationFromFileOptional("NewSong","NewSong") .. {
	-- 	ShowCommand=THEME:GetMetric(Var "LoadingScreen", "NewSongShowCommand" );
	-- 	HideCommand=THEME:GetMetric(Var "LoadingScreen", "NewSongHideCommand" );
		InitCommand=cmd(playcommand,"Set");
		BeginCommand=cmd(playcommand,"Set");
		CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
		SetCommand=function(self)
	-- 		local pTargetProfile;
			local sSong;
			-- Start!
			if GAMESTATE:GetCurrentSong() then
				if PROFILEMAN:IsSongNew(GAMESTATE:GetCurrentSong()) then
					self:playcommand("Show");
				else
					self:playcommand("Hide");
				end
			else
				self:playcommand("Hide");
			end
		end;
	};
	t[#t+1] = StandardDecorationFromFileOptional("StageDisplay","StageDisplay");
end;

if GAMESTATE:IsCourseMode() then
	t[#t+1] = StandardDecorationFromFileOptional("NumCourseSongs","NumCourseSongs")..{
		SetCommand=function(self)
			local curSelection= nil;
			local sAppend = "";
			if GAMESTATE:IsCourseMode() then
				curSelection = GAMESTATE:GetCurrentCourse();
				if curSelection then
					sAppend = (curSelection:GetEstimatedNumStages() == 1) and "Stage" or "Stages";
					self:visible(true);
					self:settext( curSelection:GetEstimatedNumStages() .. " " .. sAppend);
				else
					self:visible(false);
				end;
			else
				self:visible(false);
			end;
		end;
		CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
	};
	t[#t+1] = StandardDecorationFromFileOptional("CourseType","CourseType") .. {
		SetCommand=function(self)
			local curSelection= nil;
			if GAMESTATE:IsCourseMode() then
				curSelection = GAMESTATE:GetCurrentCourse();
				if curSelection then
					self:visible(true);
					self:settext( ToEnumShortString( curSelection:GetCourseType() ) );
				else
					self:visible(false);
				end;
			else
				self:visible(false);
			end;
		end;
		CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
	};
end

t[#t+1] = StandardDecorationFromFileOptional("DifficultyDisplay","DifficultyDisplay");
t[#t+1] = StandardDecorationFromFileOptional("SortOrderFrame","SortOrderFrame") .. {
--[[ 	BeginCommand=cmd(playcommand,"Set");
	SortOrderChangedMessageCommand=cmd(playcommand,"Set";);
	SetCommand=function(self)
		local s = SortOrderToLocalizedString( GAMESTATE:GetSortOrder() );
		self:settext( s );
		self:playcommand("Sort");
	end; --]]
};
t[#t+1] = StandardDecorationFromFileOptional("SortOrder","SortOrderText") .. {
	BeginCommand=cmd(playcommand,"Set");
	SortOrderChangedMessageCommand=cmd(playcommand,"Set";);
	SetCommand=function(self)
		local s = SortOrderToLocalizedString( GAMESTATE:GetSortOrder() );
		self:settext( s );
		self:playcommand("Sort");
	end;
};
t[#t+1] = StandardDecorationFromFileOptional("SongOptionsFrame","SongOptionsFrame") .. {
	ShowPressStartForOptionsCommand=THEME:GetMetric(Var "LoadingScreen","SongOptionsFrameShowCommand");
	ShowEnteringOptionsCommand=THEME:GetMetric(Var "LoadingScreen","SongOptionsFrameEnterCommand");
	HidePressStartForOptionsCommand=THEME:GetMetric(Var "LoadingScreen","SongOptionsFrameHideCommand");
};
t[#t+1] = StandardDecorationFromFileOptional("SongOptions","SongOptionsText") .. {
	ShowPressStartForOptionsCommand=THEME:GetMetric(Var "LoadingScreen","SongOptionsShowCommand");
	ShowEnteringOptionsCommand=THEME:GetMetric(Var "LoadingScreen","SongOptionsEnterCommand");
	HidePressStartForOptionsCommand=THEME:GetMetric(Var "LoadingScreen","SongOptionsHideCommand");
};
return t