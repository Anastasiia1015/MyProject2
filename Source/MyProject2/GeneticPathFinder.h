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
	
    virtual void Tick(float DeltaTime) override;

    // Function to define valid links between points
    void DefineLinks();

    // Function to start the genetic algorithm
    void StartGeneticAlgorithm();

    // Function to calculate the fitness of a path
    float CalculateFitness(const struct FPath& Path);

    // Function to generate a random path
    struct FPath GenerateRandomPath();

    // Function to select two parents for crossover
    void SelectParents(struct FPath& Parent1, struct FPath& Parent2);

    // Function to crossover two paths
    struct FPath Crossover(const struct FPath& Parent1, const struct FPath& Parent2);
    void VisualizePath(const FPath& Path);
    // Function to mutate a path
    void Mutate(struct FPath& Path);
    void LogPath(const FPath& Path);
    bool IsValidLink(int32 StartPoint, int32 EndPoint);
    
private:
    // Store the list of point nodes and valid links
    TArray<AActor*> PointNodes;
    TMap<int32, TArray<int32>> ValidLinks;

    // Population for the genetic algorithm
    TArray<struct FPath> Population;
    AActor* StartActor;
    AActor* EndActor;
    
};
struct FPath
{
    TArray<int32> PathPoints; // List of point indices representing the path
    float Fitness;             // Fitness of the path

    FPath() : Fitness(0.0f) {} // Default constructor
    
};