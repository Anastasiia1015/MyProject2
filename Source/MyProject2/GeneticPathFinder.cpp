// Fill out your copyright notice in the Description page of Project Settings.


#include "GeneticPathFinder.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Math/Vector.h"
#include "Math/RandomStream.h"


// Sets default values
AGeneticPathFinder::AGeneticPathFinder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    
}
const float AGeneticPathFinder::MutationRate = 0.01f;
float AGeneticPathFinder::nbits = std::log10(NumPoints) / std::log10(2);
int AGeneticPathFinder::ChromosomeLength = static_cast<int>(((nobs + 2) * nbits) / nbits);

// Called when the game starts or when spawned
void AGeneticPathFinder::BeginPlay()
{
	Super::BeginPlay();

	DefineLinks();
    
}

// Called every frame
void AGeneticPathFinder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void AGeneticPathFinder::DefineLinks()
{
    UE_LOG(LogTemp, Warning, TEXT("DefineLinks called!"));

    // Get all point nodes
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), PointNodes);

    // Filter only BP_PointNode actors (assuming you've tagged them with "Point")
    PointNodes = PointNodes.FilterByPredicate([](AActor* Actor)
        {
            return Actor->ActorHasTag("Point");
        });
    UE_LOG(LogTemp, Warning, TEXT("Found %d Point Nodes."), PointNodes.Num());

    // Get all barriers
    TArray<AActor*> Barriers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Barriers);

    // Filter for only the barriers
    Barriers = Barriers.FilterByPredicate([](AActor* Actor)
        {
            return Actor->ActorHasTag("Barrier");
        });
    UE_LOG(LogTemp, Warning, TEXT("Found %d Barriers."), Barriers.Num());

    // Find valid links
    for (int32 i = 0; i < PointNodes.Num(); i++)
    {
        for (int32 j = i + 1; j < PointNodes.Num(); j++) // Avoid redundant checks
        {
            UE_LOG(LogTemp, Warning, TEXT("Checking link between %d and %d"), i, j);
            FVector Start = PointNodes[i]->GetActorLocation();
            FVector End = PointNodes[j]->GetActorLocation();

            // Create a collision query parameters instance
            FCollisionQueryParams CollisionParams;

            // Add only the point nodes to the ignored actors list to avoid hitting them
            TArray<AActor*> IgnoredActors = { PointNodes[i], PointNodes[j] };

            // Add barriers to the collision query to ensure they can be hit by the trace
            CollisionParams.AddIgnoredActors(IgnoredActors);

            // Perform line trace between the points
            FHitResult HitResult;
            if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams))
            {
                // If we hit something, check if it's a barrier
                if (AActor* HitActor = HitResult.GetActor())
                {
                    if (HitActor->ActorHasTag("Barrier"))
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Path is blocked by Barrier: %s"), *HitActor->GetName());
                        continue; // Skip this link if blocked by a barrier
                    }
                }
            }

            // If no block was found, consider the link valid
            UE_LOG(LogTemp, Warning, TEXT("Found valid Link between %d and %d"), i, j);
            ValidLinks.FindOrAdd(i).Add(j);
            ValidLinks.FindOrAdd(j).Add(i);

            // Debug line for visualization
            DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 10.0f);
        }
    }
    for (const TPair<int32, TArray<int32>>& LinkPair : ValidLinks)
    {
        FString LinkList = FString::Printf(TEXT("Point %d is linked to: "), LinkPair.Key);

        // Iterate over the array of linked points and append them to the string
        for (int32 Link : LinkPair.Value)
        {
            LinkList += FString::Printf(TEXT("%d "), Link);
        }

        // Log the result
        UE_LOG(LogTemp, Warning, TEXT("%s"), *LinkList);
    }

}

TArray<TArray<int32>> AGeneticPathFinder::PerformGeneticOperations(
    const TArray<float>& PopulationFitness,
    const TArray<TArray<int32>>& RankedPopulation,
    const TArray<int32>& BestFitnessIndices,
    const TArray<TArray<int32>>& LastPopulation)
{
    TArray<TArray<int32>> CrossoverPopulation = PerformCrossover(RankedPopulation, BestFitnessIndices, LastPopulation);
    TArray<TArray<int32>> MutatedPopulation = PerformMutation(CrossoverPopulation);
    return MutatedPopulation;
}

// Crossover logic
TArray<TArray<int32>> AGeneticPathFinder::PerformCrossover(
    const TArray<TArray<int32>>& RankedPopulation,
    const TArray<int32>& BestFitnessIndices,
    const TArray<TArray<int32>>& LastPopulation)
{
    TArray<TArray<int32>> CrossoverPopulation;
    CrossoverPopulation.SetNum(PopulationMax);

    // Preserve the best chromosomes (elitism)
    CrossoverPopulation[0] = LastPopulation[BestFitnessIndices[0]];
    CrossoverPopulation[1] = LastPopulation[BestFitnessIndices[1]];
    CrossoverPopulation[2] = LastPopulation[BestFitnessIndices[2]];

    int32 Index = 3;

    // Generate offspring using crossover
    while (Index < PopulationMax / 5)
    {
        int32 ParentAIndex = FMath::RandRange(0, ChromosomeLength - 1);
        int32 ParentBIndex = FMath::RandRange(0, ChromosomeLength - 1);

        const TArray<int32>& ParentA = RankedPopulation[ParentAIndex];
        const TArray<int32>& ParentB = RankedPopulation[ParentBIndex];

        int32 JoiningPoint = FMath::RandRange(0, ChromosomeLength - 1);

        TArray<int32> Offspring1, Offspring2;
        Offspring1.SetNum(ChromosomeLength);
        Offspring2.SetNum(ChromosomeLength);

        // Combine segments
        for (int32 i = 0; i < JoiningPoint; i++)
        {
            Offspring1[i] = ParentA[i];
            Offspring2[i] = ParentB[i];
        }
        for (int32 i = JoiningPoint; i < ChromosomeLength; i++)
        {
            Offspring1[i] = ParentB[i];
            Offspring2[i] = ParentA[i];
        }

        CrossoverPopulation[Index++] = Offspring1;
        CrossoverPopulation[Index++] = Offspring2;
    }

    // Fill remaining with ranked population
    while (Index < PopulationMax)
    {
        CrossoverPopulation[Index] = RankedPopulation[Index];
        Index++;
    }

    return CrossoverPopulation;
}

// Mutation logic
TArray<TArray<int32>> AGeneticPathFinder::PerformMutation(const TArray<TArray<int32>>& Population)
{
    TArray<TArray<int32>> MutatedPopulation = Population;

    for (int32 i = 3; i < PopulationMax; i++) // Skip the elite chromosomes
    {
        for (int32 j = 0; j < ChromosomeLength; j++)
        {
            if (FMath::FRand() < MutationRate)
            {
                MutatedPopulation[i][j] = FMath::RandRange(1, NumPoints - 2); // Mutate within valid range
            }
        }
    }

    return MutatedPopulation;
}
float AGeneticPathFinder::CalculateDistance(const FVector& Point1, const FVector& Point2)
{
    return FVector::Dist(Point1, Point2);
}
TArray<float> AGeneticPathFinder::CalculateConsecutiveDistances(const TArray<TArray<int32>>& Population)
{
    TArray<float> ConsecutiveDistances;

    for (const TArray<int32>& Chromosome : Population)
    {
        float TotalDistance = 0.0f;

        for (int i = 0; i < Chromosome.Num() - 1; i++)
        {
            FVector Point1 = PointNodes[Chromosome[i]]->GetActorLocation();
            FVector Point2 = PointNodes[Chromosome[i + 1]]->GetActorLocation();
            TotalDistance += CalculateDistance(Point1, Point2);
        }

        ConsecutiveDistances.Add(TotalDistance);
    }

    return ConsecutiveDistances;
}
TArray<float> AGeneticPathFinder::CalculateFitnessBasedOnDistance(const TArray<float>& ConsecutiveDistances)
{
    TArray<float> FitnessBasedOnDistance;

    for (float Distance : ConsecutiveDistances)
    {
        // We can scale the fitness to make it more significant (you can adjust this value)
        FitnessBasedOnDistance.Add(10.0f * (1.0f / Distance));
    }

    return FitnessBasedOnDistance;
}

TArray<int32> AGeneticPathFinder::CalculateConnections(const TArray<TArray<int32>>& Population)
{
    TArray<int32> Connections;

    for (const TArray<int32>& Chromosome : Population)
    {
        int32 ConnectionCount = 0;

        for (int i = 0; i < Chromosome.Num() - 1; i++)
        {
            int32 A = Chromosome[i];
            int32 B = Chromosome[i + 1];

            if (ValidLinks.Contains(A) && ValidLinks[A].Contains(B))
            {
                ConnectionCount++;
            }
        }

        Connections.Add(ConnectionCount);
    }

    return Connections;
}
TArray<float> AGeneticPathFinder::CalculateFitnessBasedOnConnections(const TArray<int32>& Connections)
{
    TArray<float> FitnessBasedOnConnections;

    for (int32 ConnectionCount : Connections)
    {
        // Normalize the fitness based on the number of connections
        FitnessBasedOnConnections.Add(static_cast<float>(ConnectionCount) / (ChromosomeLength - 1));
    }

    return FitnessBasedOnConnections;
}
TArray<float> AGeneticPathFinder::CalculateFinalFitness(const TArray<float>& FitnessBasedOnDistance, const TArray<float>& FitnessBasedOnConnections)
{
    TArray<float> FinalFitness;

    for (int32 i = 0; i < FitnessBasedOnDistance.Num(); i++)
    {
        FinalFitness.Add(FitnessBasedOnDistance[i] + FitnessBasedOnConnections[i]);
    }

    return FinalFitness;
}
TArray<int32> AGeneticPathFinder::GetBestFitnessIndices(const TArray<float>& FitnessValues)
{
    TArray<int32> BestFitnessIndices;

    // Copy the fitness values to avoid modifying the original array
    TArray<float> TempFitnessValues = FitnessValues;

    while (BestFitnessIndices.Num() < 3)
    {
        // Find the index of the maximum fitness value
        int32 MaxIndex = TempFitnessValues.IndexOfByPredicate([&](float Value) { return Value == FMath::Max<float>(TempFitnessValues); });

        BestFitnessIndices.Add(MaxIndex);
        TempFitnessValues[MaxIndex] = 0; // Prevent selecting the same individual twice
    }

    return BestFitnessIndices;
}
TArray<float> AGeneticPathFinder::EvaluateFitness(const TArray<TArray<int32>>& Population)
{
    // Step 1: Calculate distances between consecutive points
    TArray<float> ConsecutiveDistances = CalculateConsecutiveDistances(Population);

    // Step 2: Calculate fitness based on total distance
    TArray<float> FitnessBasedOnDistance = CalculateFitnessBasedOnDistance(ConsecutiveDistances);

    // Step 3: Calculate the number of valid connections
    TArray<int32> Connections = CalculateConnections(Population);

    // Step 4: Calculate fitness based on the number of valid connections
    TArray<float> FitnessBasedOnConnections = CalculateFitnessBasedOnConnections(Connections);

    // Step 5: Combine the two fitness components
    TArray<float> FinalFitness = CalculateFinalFitness(FitnessBasedOnDistance, FitnessBasedOnConnections);

    // Step 6: Get the best fitness indices
    TArray<int32> BestFitnessIndices = GetBestFitnessIndices(FinalFitness);

    // Return the final fitness values and the best indices
    return FinalFitness;
}

TArray<TArray<int32>> AGeneticPathFinder::InitializePopulation()
{
    TArray<TArray<int32>> Population;

    // Call DefineLinks, but don't expect it to return anything
    DefineLinks();

    // Now use ValidLinks directly, since it's populated by DefineLinks
    TArray<float> LinkFitness = CalculateLinkDistance(ValidLinks);
    TArray<float> LinkProbability = CalculateLinkProbability(LinkFitness);

    // Calculate cumulative probability
    TArray<TArray<float>> CumulativeLinkProbability;
    CalculateCumulativeProbability(LinkProbability, CumulativeLinkProbability);

    // Create the population based on the cumulative probability
    CreatePopulation(CumulativeLinkProbability, Population);

    return Population;
}

TArray<float> AGeneticPathFinder::CalculateLinkDistance(const TMap<int32, TArray<int32>>& Links)
{
    TArray<float> LinkDistances;

    // Iterate over each entry in the ValidLinks TMap
    for (const TPair<int32, TArray<int32>>& LinkPair : Links)
    {
        int32 PointIndex = LinkPair.Key;
        const TArray<int32>& LinkedPoints = LinkPair.Value;

        // Now for each linked point in this TArray, calculate the distance
        for (int32 LinkedPoint : LinkedPoints)
        {
            // Make sure we are not comparing the same point to itself
            if (PointIndex != LinkedPoint)
            {
                // Get the locations of the points
                FVector Point1 = PointNodes[PointIndex]->GetActorLocation();
                FVector Point2 = PointNodes[LinkedPoint]->GetActorLocation();

                // Calculate the distance between the points
                float Distance = FVector::Dist(Point1, Point2);

                // Add the distance to the result array
                LinkDistances.Add(Distance);
            }
        }
    }

    return LinkDistances;
}

TArray<float> AGeneticPathFinder::CalculateLinkProbability(const TArray<float>& LinkFitness)
{
    TArray<float> LinkProbability;
    float Sum = 0.0f;

    // Calculate total fitness
    for (float Fitness : LinkFitness)
    {
        Sum += Fitness;
    }

    // Calculate probability for each link
    for (float Fitness : LinkFitness)
    {
        LinkProbability.Add(Fitness / Sum);
    }

    return LinkProbability;
}

void AGeneticPathFinder::CalculateCumulativeProbability(const TArray<float>& LinkProbability, TArray<TArray<float>>& CumulativeProbability)
{
    TArray<float> CumProb;
    float CumulativeSum = 0.0f;

    for (float Probability : LinkProbability)
    {
        CumulativeSum += Probability;
        CumProb.Add(CumulativeSum);
    }
    CumulativeProbability.Add(CumProb);
}
void AGeneticPathFinder::CreatePopulation(const TArray<TArray<float>>& LinkCumProbability, TArray<TArray<int32>>& Population)
{
    Population.SetNum(PopulationMax);

    // Find StartPoint and EndPoint indices from tags
    int32 StartIndex = -1;
    int32 EndIndex = -1;

    // Find the StartPoint and EndPoint actors and get their indices
    for (int32 i = 0; i < PointNodes.Num(); i++)
    {
        if (PointNodes[i]->ActorHasTag("StartPoint"))
        {
            StartIndex = i;
        }
        if (PointNodes[i]->ActorHasTag("EndPoint"))
        {
            EndIndex = i;
        }
    }

    // Ensure valid StartIndex and EndIndex
    if (StartIndex == -1 || EndIndex == -1)
    {
        UE_LOG(LogTemp, Error, TEXT("StartPoint or EndPoint not found!"));
        return;
    }

    // Create the population using the ValidLinks
    for (int32 k = 0; k < PopulationMax; k++)
    {
        TArray<int32> Chromosome;
        Chromosome.SetNum(ChromosomeLength);

        int32 i = StartIndex;
        int32 j = StartIndex + 1;

        // Initialize the first point in the chromosome
        Chromosome[0] = StartIndex;

        while (j < ChromosomeLength)
        {
            // Select the next point based on the valid links
            if (ValidLinks.Contains(i))
            {
                const TArray<int32>& LinkedPoints = ValidLinks[i];
                if (LinkedPoints.Num() > 0)
                {
                    // Randomly select a linked point (to simulate crossover/mutation)
                    int32 RandomIndex = FMath::RandRange(0, LinkedPoints.Num() - 1);
                    int32 NextPoint = LinkedPoints[RandomIndex];

                    Chromosome[j] = NextPoint;

                    // Move to the next point
                    i = NextPoint;
                }
            }

            // Check if we have reached the end point
            if (i == EndIndex)
            {
                // If we reached the end, set the remaining points in the chromosome to EndIndex
                while (j < ChromosomeLength)
                {
                    Chromosome[j] = EndIndex;
                    j++;
                }
                break;
            }

            j++;
        }

        Population[k] = Chromosome;
    }
}
TArray<float> AGeneticPathFinder::CalculateProbabilities(const TArray<float>& Fitness)
{
    TArray<float> Probabilities;
    float Sum = 0.0f;

    // Calculate total fitness sum
    for (float Value : Fitness)
    {
        Sum += Value;
    }

    // Calculate the probability for each chromosome based on fitness
    for (float Value : Fitness)
    {
        Probabilities.Add(Value / Sum);
    }

    return Probabilities;
}
TArray<int32> AGeneticPathFinder::RankingBasedOnRouletteWheelSelection(const TArray<float>& CumulativeProbabilities)
{
    TArray<int32> Rank;
    TArray<float> RandomNumbers;

    // Generate random numbers for roulette wheel selection
    for (int32 i = 0; i < PopulationMax; i++)
    {
        RandomNumbers.Add(FMath::FRand());
    }

    TArray<int32> SelectedChromosomes;
    for (float RandomNumber : RandomNumbers)
    {
        for (int32 i = 0; i < PopulationMax; i++)
        {
            if (RandomNumber <= CumulativeProbabilities[i])
            {
                SelectedChromosomes.Add(i);
                break;
            }
        }
    }

    // Now count how many times each chromosome is selected and rank them
    TArray<int32> RankCounts;
    RankCounts.SetNumZeroed(PopulationMax);

    for (int32 ChromosomeIndex : SelectedChromosomes)
    {
        RankCounts[ChromosomeIndex]++;
    }

    // Assign ranks based on the selection count
    for (int32 i = 0; i < RankCounts.Num(); i++)
    {
        Rank.Add(RankCounts[i]);
    }

    return Rank;
}
TArray<TArray<int32>> AGeneticPathFinder::GenerateMatingPool(const TArray<int32>& Rank, const TArray<TArray<int32>>& Population)
{
    TArray<TArray<int32>> MatingPool;

    // Generate mating pool based on the chromosome ranks
    for (int32 i = 0; i < PopulationMax; i++)
    {
        int32 RankIndex = Rank[i];
        MatingPool.Add(Population[RankIndex]);
    }

    return MatingPool;
}

TArray<TArray<int32>> AGeneticPathFinder::Ranking(const TArray<float>& Fitness, const TArray<TArray<int32>>& Population)
{
    // Step 1: Calculate probabilities for each chromosome
    TArray<float> Probabilities = CalculateProbabilities(Fitness);

    // Step 2: Calculate cumulative probability
    TArray<float> CumulativeProbabilities;
    float CumSum = 0.0f;
    for (float Prob : Probabilities)
    {
        CumSum += Prob;
        CumulativeProbabilities.Add(CumSum);
    }

    // Step 3: Perform ranking based on roulette wheel selection
    TArray<int32> Rank = RankingBasedOnRouletteWheelSelection(CumulativeProbabilities);

    // Step 4: Generate the mating pool
    TArray<TArray<int32>> MatingPool = GenerateMatingPool(Rank, Population);

    return MatingPool;
}


