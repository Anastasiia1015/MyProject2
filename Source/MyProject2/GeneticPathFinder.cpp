#include "GeneticPathFinder.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Math/Vector.h"
#include "Math/RandomStream.h"
#include "Algo/Unique.h"
#include "Containers/Array.h"

// Defines maximum population size and mutation rate
#define POPULATION_SIZE 20
#define MUTATION_RATE 0.05f
#define MAX_GENERATIONS 1000

// Sets default values
AGeneticPathFinder::AGeneticPathFinder()
{
    PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGeneticPathFinder::BeginPlay()
{
    Super::BeginPlay();
    TArray<AActor*> StartActors;
    TArray<AActor*> EndActors;

    // Find actors with tags "StartPoint" and "EndPoint"
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), "StartPoint", StartActors);
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), "EndPoint", EndActors);

    StartActor = StartActors[0]; // Take the first actor (assuming only one "StartPoint")
    EndActor = EndActors[0]; // Take the first actor (assuming only one "EndPoint")
    DefineLinks();
    StartGeneticAlgorithm();
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
                //DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 10.0f);
            }
        }
        for (int i = 0; i < ValidLinks.Num(); i++) {
            ValidLinks[i].Sort();
            ValidLinks[i].SetNum(Algo::Unique(ValidLinks[i]));  // Remove duplicates
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

// Fitness Function: Determines how good a path is
float AGeneticPathFinder::CalculateFitness(const FPath& Path)
{
    if (Path.PathPoints.Num() < 2)
        return 0.0f;

    // Get Start and End points
   
    //if (StartActors.Num() == 0 || EndActors.Num() == 0)
    //{
    //    UE_LOG(LogTemp, Warning, TEXT("StartPoint or EndPoint actors not found!"));
    //    return 0.0f; // Return a default value or handle the error as appropriate
    //}

    FVector StartLocation = StartActor->GetActorLocation();
    FVector EndLocation = EndActor->GetActorLocation();

    // Calculate path length and deviation from goal
    float PathLength = 0.0f;
    for (int i = 0; i < Path.PathPoints.Num() - 1; ++i)
    {
        FVector Current = PointNodes[Path.PathPoints[i]]->GetActorLocation();
        FVector Next = PointNodes[Path.PathPoints[i + 1]]->GetActorLocation();
        PathLength += FVector::Dist(Current, Next);
    }

    // Calculate the distance from the end point
    FVector LastPoint = PointNodes[Path.PathPoints.Last()]->GetActorLocation();
    float DistanceToEnd = FVector::Dist(LastPoint, EndLocation);

    // Fitness: shorter path length and closer to the goal
    return 1.0f / (PathLength + DistanceToEnd);  // Inversely proportional to path length + distance to goal
}

// Create a random path
FPath AGeneticPathFinder::GenerateRandomPath()
{
    FPath NewPath;
    
    // Randomly select points (avoid the start and end points)
    int32 StartIndex = PointNodes.IndexOfByKey(StartActor);
    int32 EndIndex = PointNodes.IndexOfByKey(EndActor);

    
    if (StartIndex == INDEX_NONE || EndIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Error, TEXT("Start or End actor not found in PointNodes! StartIndex: %d, EndIndex: %d"), StartIndex, EndIndex);
        return NewPath;  // Handle the error (possibly exit early)
    }

    UE_LOG(LogTemp, Warning, TEXT("Generating path from Start Index: %d to End Index: %d"), StartIndex, EndIndex);

    int32 CurrentIndex = StartIndex;
    TSet<int32> VisitedPoints; // Track points that have already been added to the path
    VisitedPoints.Add(StartIndex);
    NewPath.PathPoints.Add(StartIndex);
    while (CurrentIndex != EndIndex)
    {
        TArray<int32>& Links = ValidLinks.FindChecked(CurrentIndex);

        if (Links.Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("No valid links found for point %d!"), CurrentIndex);
            break;
        }

        // Filter out already visited points
        TArray<int32> UnvisitedLinks = Links.FilterByPredicate([&](int32 Point) {
            return !VisitedPoints.Contains(Point);
            });

        if (UnvisitedLinks.Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("No unvisited links available for point %d! Terminating."), CurrentIndex);
            break;
        }

        // Randomly select the next point from unvisited links
        int32 NextIndex = UnvisitedLinks[FMath::RandRange(0, UnvisitedLinks.Num() - 1)];

        NewPath.PathPoints.Add(NextIndex);
        VisitedPoints.Add(NextIndex); // Mark the point as visited
        
        UE_LOG(LogTemp, Warning, TEXT("Added Point %d to Path"), NextIndex);

        CurrentIndex = NextIndex; // Move to the next point
    }
    UE_LOG(LogTemp, Warning, TEXT("new generated path from generator:"));
    LogPath(NewPath);
    return NewPath;
}



// Select two paths for crossover
void AGeneticPathFinder::SelectParents(FPath& Parent1, FPath& Parent2)
{
    int32 Parent1Index = FMath::RandRange(0, Population.Num() - 1);
    int32 Parent2Index = FMath::RandRange(0, Population.Num() - 1);

    Parent1 = Population[Parent1Index];
    Parent2 = Population[Parent2Index];
}

// Crossover function
FPath AGeneticPathFinder::Crossover(const FPath& Parent1, const FPath& Parent2)
{
    FPath Child;

    // Ensure both parents have valid paths
    if (Parent1.PathPoints.Num() == 0 || Parent2.PathPoints.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("One of the parents has an empty path!"));
        return Child;  // Return empty path if either parent is invalid
    }

    // Ensure the crossover point is within bounds for both parents
    int32 CrossoverPoint = FMath::RandRange(0, FMath::Min(Parent1.PathPoints.Num(), Parent2.PathPoints.Num()) - 1);

    // Take the first part from Parent1
    Child.PathPoints.Append(Parent1.PathPoints.GetData(), CrossoverPoint);

    // Take the second part from Parent2
    Child.PathPoints.Append(Parent2.PathPoints.GetData() + CrossoverPoint, Parent2.PathPoints.Num() - CrossoverPoint);

    // Optionally, enforce valid links in the child path
    for (int32 i = 0; i < Child.PathPoints.Num() - 1; i++)
    {
        int32 StartPoint = Child.PathPoints[i];
        int32 EndPoint = Child.PathPoints[i + 1];

        // Ensure that the link between points is valid
        if (!IsValidLink(StartPoint, EndPoint))
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid link detected between %d and %d, fixing..."), StartPoint, EndPoint);
            // Fix the invalid link by choosing a valid link
            const TArray<int32>* ValidStartLinks = ValidLinks.Find(StartPoint);
            if (ValidStartLinks && ValidStartLinks->Num() > 0)
            {
                // Pick a valid link
                EndPoint = (*ValidStartLinks)[FMath::RandRange(0, ValidStartLinks->Num() - 1)];
                Child.PathPoints[i + 1] = EndPoint;  // Update the path
            }
        }
    }

    // Explicitly set the start and end points for the child path
    if (Child.PathPoints.Num() > 0)
    {
        Child.PathPoints[0] = Parent1.PathPoints[0];  // Ensure the start point is from Parent1
        Child.PathPoints[Child.PathPoints.Num() - 1] = Parent2.PathPoints[Parent2.PathPoints.Num() - 1];  // Ensure the end point is from Parent2
    }

    // Optionally, handle the case where the child path is incomplete or invalid
    if (Child.PathPoints.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Child path is empty after crossover!"));
    }

    return Child;
}




// Mutation function: Randomly change part of the path
void AGeneticPathFinder::Mutate(FPath& Path)
{
    UE_LOG(LogTemp, Warning, TEXT("Mutation started"));
    float RandValue = FMath::FRand();
    UE_LOG(LogTemp, Warning, TEXT("Random Value: %f"), RandValue);

    if (RandValue < MUTATION_RATE)
    {
        // Ensure there are points to mutate (avoid the start and end points)
        if (Path.PathPoints.Num() <= 2)  // At least 2 points (start and end)
        {
            UE_LOG(LogTemp, Warning, TEXT("Mutation aborted: Path has too few points."));
            return;
        }

        UE_LOG(LogTemp, Warning, TEXT("Mutation started 2.0"));

        // Select a random mutation point (excluding start and end points)
        int32 MutationPoint = FMath::RandRange(1, Path.PathPoints.Num() - 2);  // Skip start (0) and end (Num()-1)
        UE_LOG(LogTemp, Warning, TEXT("MutationPoint selected: %d"), MutationPoint);

        int32 MutationIndex = Path.PathPoints[MutationPoint];
        UE_LOG(LogTemp, Warning, TEXT("MutationIndex selected: %d"), MutationIndex);

        // Ensure valid links exist for the mutation index
        const TArray<int32>* Links = ValidLinks.Find(MutationIndex);
        if (Links == nullptr || Links->Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("Mutation point %d has no valid links or links array is empty!"), MutationIndex);
            return;
        }

        // Find a valid link for mutation
        bool bValidLinkFound = false;
        int32 NewPoint = -1;

        for (int32 i = 0; i < Links->Num(); i++)
        {
            int32 CandidatePoint = (*Links)[i];
            // Ensure that this link is valid
            if (IsValidLink(MutationIndex, CandidatePoint))
            {
                NewPoint = CandidatePoint;
                bValidLinkFound = true;
                break;
            }
        }

        if (!bValidLinkFound)
        {
            UE_LOG(LogTemp, Warning, TEXT("No valid links found for mutation point %d!"), MutationIndex);
            return;
        }

        // Apply the mutation
        Path.PathPoints[MutationPoint] = NewPoint;
        UE_LOG(LogTemp, Warning, TEXT("Mutated Path at Point %d to %d"), MutationPoint, NewPoint);
    }
}




// The main Genetic Algorithm
void AGeneticPathFinder::StartGeneticAlgorithm()
{
    int StagnationCount = 0;  // Counter for generations without improvement
    const int MaxStagnationCount = 50;  // Number of generations with no improvement to allow before stopping

    // Initialize population with random paths
    for (int i = 0; i < POPULATION_SIZE; i++)
    {
        FPath newPath = GenerateRandomPath();
        UE_LOG(LogTemp, Warning, TEXT("new generated path:"));
        LogPath(newPath);
        Population.Add(newPath);
    }

    // Evolve population over generations
    for (int Gen = 0; Gen < MAX_GENERATIONS; Gen++)
    {
        // Calculate fitness for each individual
        for (FPath& Path : Population)
        {
            Path.Fitness = CalculateFitness(Path);
            UE_LOG(LogTemp, Warning, TEXT("CalculateFitness called"));
        }

        // Sort the population by fitness (best first)
        Population.Sort([](const FPath& A, const FPath& B) { return A.Fitness > B.Fitness; });

        // Track fitness changes
        static float PreviousBestFitness = 0.0f;
        float CurrentBestFitness = Population[0].Fitness;

        if (CurrentBestFitness == PreviousBestFitness)
        {
            StagnationCount++;  // Increment stagnation count if the fitness hasn't improved
        }
        else
        {
            StagnationCount = 0;  // Reset stagnation count if there's an improvement
        }

        // If the best fitness hasn't improved for a set number of generations, stop
        if (StagnationCount >= MaxStagnationCount)
        {
            UE_LOG(LogTemp, Warning, TEXT("Stopping due to stagnation (no improvement in best fitness for %d generations)."), MaxStagnationCount);
            LogPath(Population[0]);
            VisualizePath(Population[0]);
            break;
        }

        // Store the best fitness for the next generation check
        PreviousBestFitness = CurrentBestFitness;

        // Generate next generation
        TArray<FPath> NewGeneration;
        UE_LOG(LogTemp, Warning, TEXT("NewGeneration created"));
        UE_LOG(LogTemp, Warning, TEXT("population[0]:"));
        LogPath(Population[0]);
        UE_LOG(LogTemp, Warning, TEXT("population[1]:"));
        LogPath(Population[1]);

        // Elitism: Keep the top 2 paths
        NewGeneration.Add(Population[0]);
        UE_LOG(LogTemp, Warning, TEXT("NewGeneration.Add(Population[0]);"));
        NewGeneration.Add(Population[1]);
        UE_LOG(LogTemp, Warning, TEXT("NewGeneration.Add(Population[1]);"));

        // Create new paths by crossover and mutation
        while (NewGeneration.Num() < POPULATION_SIZE)
        {
            FPath Parent1, Parent2;
            UE_LOG(LogTemp, Warning, TEXT("Before SelectParents"));
            SelectParents(Parent1, Parent2);
            UE_LOG(LogTemp, Warning, TEXT("Before crossover"));
            FPath Child = Crossover(Parent1, Parent2);
            UE_LOG(LogTemp, Warning, TEXT("Child after crossover:"));
            LogPath(Child);
            UE_LOG(LogTemp, Warning, TEXT("Before child mutated:"));
            Mutate(Child);
            UE_LOG(LogTemp, Warning, TEXT("Child after mutation:"));
            LogPath(Child);

            NewGeneration.Add(Child);
            UE_LOG(LogTemp, Warning, TEXT("NewGeneration.Add(Child);"));
        }

        // Replace the old population with the new one
        UE_LOG(LogTemp, Warning, TEXT("before new gen"));
        Population = NewGeneration;
        UE_LOG(LogTemp, Warning, TEXT("after new gen"));
        UE_LOG(LogTemp, Warning, TEXT("Generation %d: Best Fitness = %f"), Gen, Population[0].Fitness);
    }
}

void AGeneticPathFinder::VisualizePath(const FPath& Path)
{
    if (Path.PathPoints.Num() < 2)
    {
        return; // No valid path to visualize
    }

    // Iterate through the path points and draw debug lines
    for (int32 i = 0; i < Path.PathPoints.Num() - 1; i++)
    {
        // Get the actor from PointNodes using Path.PathPoints[i], which is an index
        TObjectPtr<AActor> ActorStartPtr = PointNodes[Path.PathPoints[i]];
        TObjectPtr<AActor> ActorEndPtr = PointNodes[Path.PathPoints[i + 1]];

        // Check if the actors are valid by checking if the pointers are non-null
        if (ActorStartPtr && ActorEndPtr)
        {
            // Dereference the TObjectPtr to get the raw AActor pointer
            AActor* ActorStart = ActorStartPtr.Get();
            AActor* ActorEnd = ActorEndPtr.Get();

            if (ActorStart && ActorEnd)
            {
                FVector Start = ActorStart->GetActorLocation();
                FVector End = ActorEnd->GetActorLocation();

                // Draw a line between the points
                DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 10.0f, 0, 1);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid actor at path points %d and %d"), i, i + 1);
        }
    }
}
void AGeneticPathFinder::LogPath(const FPath& Path)
{
    FString PathString = TEXT("");

    // Iterate through the points in the path and log them
    for (int32 i = 0; i < Path.PathPoints.Num(); i++)
    {
        PathString += FString::Printf(TEXT("%d"), Path.PathPoints[i]);

        if (i < Path.PathPoints.Num() - 1)
        {
            PathString += TEXT(" -> ");
        }
    }

    // Log the path to the output
    UE_LOG(LogTemp, Warning, TEXT("%s"), *PathString);
}
bool AGeneticPathFinder::IsValidLink(int32 StartPoint, int32 EndPoint)
{
    // Check if there is a valid link between the points
    const TArray<int32>* ValidLinksForStart = ValidLinks.Find(StartPoint);
    if (ValidLinksForStart && ValidLinksForStart->Contains(EndPoint))
    {
        return true;  // Link is valid
    }

    return false;  // Link is invalid
}
