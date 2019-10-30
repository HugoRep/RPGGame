// Fill out your copyright notice in the Description page of Project Settings.

#include "MyTraceActor.h"

#include "Materials/MaterialExpressionTextureSampleParameter2D.h"

#include "Modules/ModuleManager.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "PixelFormat.h"
#include "RenderUtils.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Async/ParallelFor.h"


// Sets default values
AMyTraceActor::AMyTraceActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyTraceActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyTraceActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


namespace {
	
	static UTexture2D* createTex(int32 InSizeX, int32 InSizeY, FString name, UPackage *package) {
		auto format = PF_B8G8R8A8;
		UTexture2D* NewTexture = NULL;
		if (InSizeX > 0 && InSizeY > 0 &&
			(InSizeX % GPixelFormats[format].BlockSizeX) == 0 &&
			(InSizeY % GPixelFormats[format].BlockSizeY) == 0)
		{
			if (package == GetTransientPackage()) {
				NewTexture = NewObject<UTexture2D>(GetTransientPackage(), NAME_None, EObjectFlags::RF_Public | RF_Transient);
			}
			else {
				NewTexture = NewObject<UTexture2D>(
					// GetTransientPackage(),
					package,
					*name,
					//RF_Transient
					RF_Public | RF_Standalone
					);
			}

			NewTexture->PlatformData = new FTexturePlatformData();
			NewTexture->PlatformData->SizeX = InSizeX;
			NewTexture->PlatformData->SizeY = InSizeY;
			NewTexture->PlatformData->PixelFormat = format;

			int32 NumBlocksX = InSizeX / GPixelFormats[format].BlockSizeX;
			int32 NumBlocksY = InSizeY / GPixelFormats[format].BlockSizeY;
			FTexture2DMipMap* Mip = new(NewTexture->PlatformData->Mips) FTexture2DMipMap();
			Mip->SizeX = InSizeX;
			Mip->SizeY = InSizeY;
			Mip->BulkData.Lock(LOCK_READ_WRITE);
			Mip->BulkData.Realloc(NumBlocksX * NumBlocksY * GPixelFormats[format].BlockBytes);
			Mip->BulkData.Unlock();
		}
		else
		{
			UE_LOG(LogTexture, Warning, TEXT("Invalid parameters specified for UTexture2D::Create()"));
		}
		return NewTexture;
	}

	struct MyRay {
		FVector Start;
		FVector End;
	};


	static FVector GetColor(FHitResult &ret, UObject *world, int &count, int countMax) {
		count++;
		if (count >= countMax) {
			return FVector(0);
		}
		FVector BaseColor(1);

		BaseColor = FVector(0.73f);

		bool Chara = true;
		if (ret.Actor->GetName().Find(TEXT("red")) >= 0) {
			BaseColor.Set(0.05f, 0.05f, 0.65f);
			Chara = false;
		}
		if (ret.Actor->GetName().Find(TEXT("green")) >= 0) {
			BaseColor.Set(0.05f, 0.65f, 0.05f);
			//BaseColor.Set(0.12f, 0.45f, 0.15f);
			Chara = false;
		}

		if (ret.Actor->GetName().Find(TEXT("light")) >= 0) {
			BaseColor = FVector(15.f);
			Chara = false;
			return BaseColor;
		}

		FVector Start = ret.Location;

		FVector End(100);
		while (End.SizeSquared() > 1) {
			End.Set(FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f));
		}
		End += ret.Normal;

		if (1) {
			//FVector v = (ret.TraceEnd - ret.TraceStart).GetSafeNormal();
			//End = v - 2.f * FVector::DotProduct(v, ret.Normal)*ret.Normal;
		}
		/*
		if (Chara == false){
			FVector2D uv;
			UGameplayStatics::FindCollisionUV(ret, 0, uv);

			uv *= 10.f;

			int i = FMath::FloorToInt(uv.X) + FMath::FloorToInt(uv.Y);

			if (i % 2) {
				BaseColor.Set(0, 0, 1);
			}
			else {
				BaseColor.Set(0, 1, 0);
			}
		}
		*/
		End.Normalize();

		Start += End * 0.1f;
		End = End * 10000.f + Start;

		TArray<AActor*> ActorsToIgnore;
		FHitResult OutHit;

		bool b = UKismetSystemLibrary::LineTraceSingle(world, Start, End,
			ETraceTypeQuery::TraceTypeQuery1, true,
			ActorsToIgnore, EDrawDebugTrace::None, OutHit,
			true);

		if (b) {
			return GetColor(OutHit, world, count, countMax) * BaseColor;
		}

		return FVector(0.f);
	}
}


void AMyTraceActor::TraceRender() {

	if (Width * Height * DrawSize == 0) {
		return;
	}

	UTexture2D* NewTexture2D = createTex(Width, Height, FString(TEXT("T_")), GetTransientPackage());

	uint8* MipData = (uint8*)NewTexture2D->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);

	ParallelFor(Height, [&]( int32 y)
	//for (int32 y = 0; y < Height; y++)
	{
		//const aiTexel *c = &(t.pcData[y*Width]);
		uint8* DestPtr = &MipData[y * Width * sizeof(FColor)];
		for (int32 x = 0; x < Width; x++)
		{
			DestPtr[0] = 0;
			DestPtr[1] = 0;
			DestPtr[2] = 0;
			DestPtr[3] = 255;

			uint8 *p = DestPtr;
			{
				//bool UKismetSystemLibrary::LineTraceSingle(UObject* WorldContextObject, const FVector Start, const FVector End, ETraceTypeQuery TraceChannel, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, EDrawDebugTrace::Type DrawDebugType, FHitResult& OutHit, bool bIgnoreSelf, FLinearColor TraceColor = FLinearColor::Red, FLinearColor TraceHitColor = FLinearColor::Green, float DrawTime = 5.0f);
				FVector Start = this->GetActorTransform().GetLocation();
				//Start += FVector(0.f, DrawSize/Width * x, -DrawSize/Height * y);

				FVector EndBase = FVector(DrawSize, DrawSize*((float)x/Width -0.5f), DrawSize*(1.f- (float)y/Height-0.5f));
				{
					EndBase = this->GetActorForwardVector() * DrawSize;

					EndBase += this->GetActorRightVector() * DrawSize*((float)x / Width - 0.5f);
					EndBase += this->GetActorUpVector() * DrawSize*(1.f - (float)y / Height - 0.5f);
				}


				TArray<AActor*> ActorsToIgnore;
				FHitResult OutHit;

				int rayCount = 0;
				FVector col(0);


				{
					for (int i = 0; i < SampleNum; ++i) {
						FVector d1, d2;

						d1 = this->GetActorRightVector() * (FMath::FRandRange(-1, 1)) * DrawSize/Width  / 2;
						d2 = this->GetActorUpVector() * (FMath::FRandRange(-1, 1)) * DrawSize/Height  / 2;
						FVector End = EndBase + d1 + d2;
						End = Start + End.GetSafeNormal() * 100000.f;

						bool ret = UKismetSystemLibrary::LineTraceSingle(this, Start, End,
							ETraceTypeQuery::TraceTypeQuery1, true,
							ActorsToIgnore, EDrawDebugTrace::None, OutHit,
							true);
						if (ret) {
							int ccc = 0;
							FVector c = GetColor(OutHit, this, ccc, countMax);
							c.X = FMath::Clamp(c.X, 0.f, 1.f);
							c.Y = FMath::Clamp(c.Y, 0.f, 1.f);
							c.Z = FMath::Clamp(c.Z, 0.f, 1.f);

							col += c;

							rayCount++;
						}
					}
				}

				if (rayCount) {

					col /= rayCount;
					//FVector col = GetColor(OutHit, this, c);

					col.X = FMath::Pow(col.X, 0.4545);
					col.Y = FMath::Pow(col.Y, 0.4545);
					col.Z = FMath::Pow(col.Z, 0.4545);

					col *= 255.f;

					p[0] = FMath::Clamp(col.X, 0.f, 255.f);
					p[1] = FMath::Clamp(col.Y, 0.f, 255.f);
					p[2] = FMath::Clamp(col.Z, 0.f, 255.f);

					//p[0] = (uint8)FMath::Abs(OutHit.Normal.X * 255);
					//p[1] = (uint8)FMath::Abs(OutHit.Normal.Y * 255);
					//p[2] = (uint8)FMath::Abs(OutHit.Normal.Z * 255);
					p[3] = 255;
				}
			}
			//*DestPtr++ = 0;// c->b;
			//*DestPtr++ = 0;// c->g;
			//*DestPtr++ = pp;// c->r;
			//*DestPtr++ = 255;// c->a;

			DestPtr += 4;
		}
	});





	NewTexture2D->PlatformData->Mips[0].BulkData.Unlock();

	// Set options
	NewTexture2D->SRGB = true;// bUseSRGB;
	NewTexture2D->CompressionSettings = TC_Default;
	//if (NormalBoolTable[i]) {
	//	NewTexture2D->CompressionSettings = TC_Normalmap;
	//	NewTexture2D->SRGB = 0;
	//}
	NewTexture2D->AddressX = TA_Wrap;
	NewTexture2D->AddressY = TA_Wrap;

#if WITH_EDITORONLY_DATA
	NewTexture2D->CompressionNone = true;
	NewTexture2D->MipGenSettings = TMGS_NoMipmaps;
	//NewTexture2D->Source.Init(Width, Height, 1, 1, ETextureSourceFormat::TSF_BGRA8, RawData->GetData());
#endif

	// Update the remote texture data
	NewTexture2D->UpdateResource();

	TextureToRender = NewTexture2D;
}
