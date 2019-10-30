// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyTraceActor.generated.h"

UCLASS()
class VRM4UMISC_API AMyTraceActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyTraceActor();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mesh)
	UTexture2D* TextureToRender;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mesh)
	int Width = 256;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mesh)
	int Height = 256;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mesh)
	float DrawSize = 256;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mesh)
	float SampleNum = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mesh)
	int countMax = 20;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = Miscellaneous)
	void TraceRender();
	
};
