

#pragma once

#include "GameFramework/Actor.h"

#include "Public/Simulation/DeepDriveSimulationDefines.h"
#include "Public/Server/Messages/DeepDriveMessageIds.h"

#include <map>
#include <functional>

#include "DeepDriveSimulation.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDeepDriveSimulation, Log, All);

class DeepDriveSimulationCaptureProxy;
class DeepDriveSimulationServerProxy;
class DeepDriveSimulationStateMachine;
class DeepDriveSimulationServer;

class DeepDriveSimulationRunningState;
class DeepDriveSimulationReseState;

class ADeepDriveAgent;
class ADeepDriveAgentControllerCreator;
class ADeepDriveAgentControllerBase;
class UCaptureSinkComponentBase;
class ADeepDriveSimulationFreeCamera;
class ADeepDriveSplineTrack;
class UDeepDriveRandomStream;

struct SimulationConfiguration;
struct SimulationGraphicsSettings;

namespace deepdrive { namespace server {
struct MessageHeader;
} }

USTRUCT(BlueprintType)
struct FDeepDriveRandomStreamData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	bool	ReSeedOnReset = true;

	FDeepDriveRandomStreamData()
		:	ReSeedOnReset(true)
		,	RandomStream(0)
	{	}

	FDeepDriveRandomStreamData(UDeepDriveRandomStream *randomStream, bool reseedOnReset)
		:	ReSeedOnReset(reseedOnReset)
		,	RandomStream(randomStream)
	{	}

	void setRandomStream(UDeepDriveRandomStream *randomStream)
	{
		RandomStream = randomStream;
	}

	UDeepDriveRandomStream* getRandomStream()
	{
		return RandomStream;
	}

private:

	UPROPERTY()
	UDeepDriveRandomStream		*RandomStream = 0;
};

USTRUCT(BlueprintType)
struct FDeepDriveAdditionalAgentData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	TSubclassOf<ADeepDriveAgent>	Agent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	EDeepDriveAgentControlMode		Mode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	int32	ConfigurationSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	int32	StartPositionSlot;
};

UCLASS()
class DEEPDRIVEPLUGIN_API ADeepDriveSimulation	:	public AActor
{
	friend class DeepDriveSimulationRunningState;
	friend class DeepDriveSimulationResetState;

	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	ADeepDriveSimulation();

	~ADeepDriveSimulation();

	// Called when the game starts or when spawned
	virtual void PreInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capture)
	float CaptureInterval = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SimulationServer)
	FString		SimulationIPAddress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SimulationServer)
	int32		SimulationPort = 9009;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ClientConnections)
	FString		ClientsIPAddress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ClientConnections)
	int32		ClientsPort = 9876;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Agents)
	TSubclassOf<ADeepDriveAgent>	Agent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Agents)
	EDeepDriveAgentControlMode	InitialControllerMode = EDeepDriveAgentControlMode::NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Agents)
	int32	InitialConfigurationSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Agents)
	int32	StartPositionSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Agents)
	TArray<FDeepDriveAdditionalAgentData>	AdditionalAgents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Agents)
	TMap<EDeepDriveAgentControlMode, ADeepDriveAgentControllerCreator*>	ControllerCreators;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FreeCamera)
	ADeepDriveSimulationFreeCamera	*FreeCamera = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Randomness)
	int32	Seed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Randomness)
	TMap<FName, FDeepDriveRandomStreamData>	RandomStreams;

	UFUNCTION(BlueprintCallable, Category = "Simulation")
	void ResetSimulation();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void MoveForward(float AxisValue);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void MoveRight(float AxisValue);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void LookUp(float AxisValue);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void Turn(float AxisValue);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SelectCamera(EDeepDriveAgentCameraType CameraType);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SelectMode(EDeepDriveAgentControlMode Mode);

	UFUNCTION(BlueprintCallable, Category = "Spectator")
	void NextAgent();

	UFUNCTION(BlueprintCallable, Category = "Spectator")
	void PreviousAgent();

	UFUNCTION(BlueprintImplementableEvent, Category = "Agents")
	void OnAgentSpawned(ADeepDriveAgent *SpawnedAgent);

	UFUNCTION(BlueprintImplementableEvent, Category = "Agents")
	void OnCurrentAgentChanged(ADeepDriveAgent *CurrentAgent);

	UFUNCTION(BlueprintImplementableEvent, Category = "Simulation")
	void SetDateAndTime(int32 Year, int32 Month, int32 Day, int32 Hour, int32 Minute);

	UFUNCTION(BlueprintCallable, Category = "Misc")
	void RegisterRandomStream(const FName &RandomStreamId, bool ReseedOnReset);

	UFUNCTION(BlueprintCallable, Category = "Misc")
	UDeepDriveRandomStream* GetRandomStream(const FName &RandomStreamId);

	UFUNCTION(BlueprintCallable, Category = "Agents")
	void OnDebugTrigger();

	void enqueueMessage(deepdrive::server::MessageHeader *message);

	void configure(const SimulationConfiguration &configuration, const SimulationGraphicsSettings &graphicsSettings, bool initialConfiguration);
	bool resetAgent();
	
	ADeepDriveAgent* getCurrentAgent() const;
	ADeepDriveAgentControllerBase* getCurrentAgentController() const;
	TArray<UCaptureSinkComponentBase*>& getCaptureSinks();

	void initializeAgents();

private:

	typedef TQueue<deepdrive::server::MessageHeader*> MessageQueue;

	typedef std::function< void(const deepdrive::server::MessageHeader&) > HandleMessageFuncPtr;
	typedef std::map<deepdrive::server::MessageId, HandleMessageFuncPtr>	MessageHandlers;

	bool isActive() const;

	void configure_(const deepdrive::server::MessageHeader& message);

	ADeepDriveAgent* spawnAgent(EDeepDriveAgentControlMode mode, int32 configSlot, int32 startPosSlot);

	void spawnAdditionalAgents();

	ADeepDriveAgentControllerBase* spawnController(EDeepDriveAgentControlMode mode, int32 configSlot, int32 startPosSlot);

	void switchToAgent(int32 index);
	void switchToCamera(EDeepDriveAgentCameraType type);

	void applyGraphicsSettings(const SimulationGraphicsSettings &gfxSettings);

	DeepDriveSimulationStateMachine			*m_StateMachine = 0;
	DeepDriveSimulationServer				*m_SimulationServer = 0;
	DeepDriveSimulationServerProxy			*m_ServerProxy = 0;
	DeepDriveSimulationCaptureProxy			*m_CaptureProxy = 0;
	TArray<UCaptureSinkComponentBase*>		m_CaptureSinks;

	MessageQueue							m_MessageQueue;
	MessageHandlers							m_MessageHandlers;


	TArray<ADeepDriveAgent*>				m_Agents;
	int32									m_curAgentIndex = 0;
	ADeepDriveAgent							*m_curAgent = 0;
	EDeepDriveAgentControlMode				m_curAgentMode = EDeepDriveAgentControlMode::NONE;


	ADeepDriveAgentControllerBase			*m_curAgentController = 0;

	EDeepDriveAgentCameraType				m_curCameraType = EDeepDriveAgentCameraType::NONE;
	float									m_OrbitCameraPitch = 0.0f;
	float									m_OrbitCameraYaw = 0.0f;
};

inline bool ADeepDriveSimulation::isActive() const
{
	return m_StateMachine != 0;
}

inline ADeepDriveAgent* ADeepDriveSimulation::getCurrentAgent() const
{
	return m_curAgent;
}

inline ADeepDriveAgentControllerBase* ADeepDriveSimulation::getCurrentAgentController() const
{
	return m_curAgentController;
}

inline TArray<UCaptureSinkComponentBase*>& ADeepDriveSimulation::getCaptureSinks()
{
	return m_CaptureSinks;
}
