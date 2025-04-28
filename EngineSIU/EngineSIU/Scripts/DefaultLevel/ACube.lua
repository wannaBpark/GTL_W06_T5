local ReturnTable = {}

local FVector = EngineTypes.FVector

-- BeginPlay: Actor가 처음 활성화될 때 호출
function ReturnTable:BeginPlay()

    print("BeginPlay ", self.Name)

end

-- Tick: 매 프레임마다 호출
function ReturnTable:Tick(DeltaTime)
    
    local this = self.this 

    local NewLoc = this.ActorLocation
    NewLoc.X = NewLoc.X + 1.0 * DeltaTime
    this.ActorLocation = NewLoc

end

-- EndPlay: Actor가 파괴되거나 레벨이 전환될 때 호출
function ReturnTable:EndPlay(EndPlayReason)
    -- print("[Lua] EndPlay called. Reason:", EndPlayReason)
    print("EndPlay")

end

return ReturnTable
