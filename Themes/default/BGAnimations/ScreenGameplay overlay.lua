local t = Def.ActorFrame {};
--[[ if GAMESTATE:GetCurrentCourse() then
	if GAMESTATE:GetCurrentCourse():GetCourseType() == "CourseType_Survival" then
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
return t