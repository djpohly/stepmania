local t = Def.ActorFrame {};
local function UpdateTime(self)
	local c = self:GetChildren();
	local bOni = GAMESTATE:GetCurrentCourse():GetCourseType() == "CourseType_Oni" and true or false;
	for pn in ivalues(PlayerNumber) do
		local vStats = STATSMAN:GetCurStageStats():GetPlayerStageStats( pn );
		local vTime;
		local aTotalTime = self:GetChild( string.format("RemainingTime" .. PlayerNumberToString(pn) ) );
		--
		if vStats then
			if bOni then
				vTime = vStats:GetAliveSeconds();
			else
				vTime = vStats:GetLifeRemainingSeconds();
			end;
			aTotalTime:settext( SecondsToMMSSMsMs( vTime ) );
		end;
	end;
end
if GAMESTATE:GetCurrentCourse() then
	if GAMESTATE:GetCurrentCourse():GetCourseType() == "CourseType_Survival" or "CourseType_Oni" then
		-- RemainingTime
		for pn in ivalues(PlayerNumber) do
			local MetricsName = "RemainingTime" .. PlayerNumberToString(pn);
			t[#t+1] = LoadActor( THEME:GetPathG( Var "LoadingScreen", "RemainingTime"), pn ) .. {
				InitCommand=function(self) 
					self:player(pn); 
					self:name(MetricsName); 
					ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen"); 
				end;
			};
		end
		for pn in ivalues(PlayerNumber) do
			local MetricsName = "DeltaSeconds" .. PlayerNumberToString(pn);
			t[#t+1] = LoadActor( THEME:GetPathG( Var "LoadingScreen", "DeltaSeconds"), pn ) .. {
				InitCommand=function(self) 
					self:player(pn); 
					self:name(MetricsName); 
					ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen"); 
				end;
			};
		end
	end;
end; --]]
t.InitCommand=cmd(SetUpdateFunction,UpdateTime);
return t