// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GeneticPathFinder.generated.h"

UCLASS()
class MYPROJECT2_API AGeneticPathFinder : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGeneticPathFinder();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	static const int  PopulationMax = 1000;
	static const int nobs = 1; // Number of obstacles
	static const int NumPoints = 6; // Number of path points (as given in path_points)
	static const float MutationRate;
	static float nbits;
	static int ChromosomeLength;
	
	virtual void Tick(float DeltaTime) override;
	void DefineLinks();
	TArray<AActor*> PointNodes;
	TMap<int32, TArray<int32>> ValidLinks;
	TArray<TArray<int32>> PerformGeneticOperations(
		const TArray<float>& PopulationFitness,
		const TArray<TArray<int32>>& RankedPopulation,
		const TArray<int32>& BestFitnessIndices,
		const TArray<TArray<int32>>& LastPopulation
	);
	TArray<TArray<int32>> PerformCrossover(
		const TArray<TArray<int32>>& RankedPopulation,
		const TArray<int32>& BestFitnessIndices,
		const TArray<TArray<int32>>& LastPopulation
	);
	TArray<TArray<int32>> PerformMutation(const TArray<TArray<int32>>& Population);
	float CalculateDistance(const FVector& Point1, const FVector& Point2);
	TArray<float> CalculateConsecutiveDistances(const TArray<TArray<int32>>& Population);
	TArray<float> CalculateFitnessBasedOnDistance(const TArray<float>& ConsecutiveDistances);
	TArray<int32> CalculateConnections(const TArray<TArray<int32>>& Population);
	TArray<float> CalculateFitnessBasedOnConnections(const TArray<int32>& Connections);
	TArray<float> CalculateFinalFitness(const TArray<float>& FitnessBasedOnDistance, const TArray<float>& FitnessBasedOnConnections);
	TArray<int32> GetBestFitnessIndices(const TArray<float>& FitnessValues);
	TArray<float> EvaluateFitness(const TArray<TArray<int32>>& Population);
	TArray<TArray<int32>> InitializePopulation();
	TArray<float> CalculateLinkDistance(const TMap<int32, TArray<int32>>& Links);
	void CalculateCumulativeProbability(const TArray<float>& LinkProbability, TArray<TArray<float>>& CumulativeProbability);
	void CreatePopulation(const TArray<TArray<float>>& LinkCumProbability, TArray<TArray<int32>>& Population);
	TArray<float> CalculateProbabilities(const TArray<float>& Fitness);
	TArray<float> CalculateLinkProbability(const TArray<float>& LinkFitness);
	TArray<int32> RankingBasedOnRouletteWheelSelection(const TArray<float>& CumulativeProbabilities);
	TArray<TArray<int32>> GenerateMatingPool(const TArray<int32>& Rank, const TArray<TArray<int32>>& Population);
	TArray<TArray<int32>> Ranking(const TArray<float>& Fitness, const TArray<TArray<int32>>& Population);

};
