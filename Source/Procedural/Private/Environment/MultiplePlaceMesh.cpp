#include "ProceduralPrivatePCH.h"

#include "MultiplePlaceMesh.h"

#define LOCTEXT_NAMESPACE "MultiplePlaceMesh" 

static ConstructorHelpers::FObjectFinderOptional<UTexture2D> MultiplePlaceMeshTexture = TEXT("/Engine/EditorResources/S_Actor");

AMultiplePlaceMesh::AMultiplePlaceMesh() {
    USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = SceneComponent;

#if WITH_EDITORONLY_DATA
    SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
    if (SpriteComponent)
    {
        SpriteComponent->Sprite = MultiplePlaceMeshTexture.Get(); // Get the sprite texture from helper class object
        SpriteComponent->SpriteInfo.DisplayName = LOCTEXT("Icon", "Icon");    // Assign sprite display name
        SpriteComponent->SetupAttachment(RootComponent);     // Attach sprite to scene component
    }
#endif // WITH_EDITORONLY_DATA

    //Setup Spline Component
    SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
    SplineComponent->SetupAttachment(RootComponent);
    SplineComponent->SetUnselectedSplineSegmentColor(FLinearColor(0.0f, 0.725f, 1.0f));
    SplineComponent->SetDrawDebug(false);

    //Setup Mesh Component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    MeshComponent->SetupAttachment(RootComponent);

#if WITH_EDITORONLY_DATA
    //If it's in editor, don't show main mesh
    MeshComponent->SetVisibility(false);

    //Load Mesh Preview material
    PreviewMaterial = LoadObject<UMaterial>(nullptr, TEXT("/JinkCore/Procedural/M_MeshPreview.M_MeshPreview"), nullptr, LOAD_None, nullptr);
#endif
}

void AMultiplePlaceMesh::OnConstruction(const FTransform & Transform)
{
    if (Mesh) {
        MeshComponent->SetStaticMesh(Mesh);
    }

    //Remove Old Components
    ClearPreviews();

    //For each spline point
    const int PointCount = SplineComponent->GetNumberOfSplinePoints();
    for (int i = 0; i < PointCount; i++)
    {
        //Reset every point's tangent in the spline to zero. This creates straight lines.
        SplineComponent->SetTangentAtSplinePoint(i, FVector::ZeroVector, ESplineCoordinateSpace::Local);

#if WITH_EDITORONLY_DATA
        if(MeshComponent->GetStaticMesh()) {
            //Show Mesh Preview
            UStaticMeshComponent* PreviewMesh = NewObject<UStaticMeshComponent>(this, FName(*("Preview_" + FString::FromInt(i))));
            PreviewMesh->RegisterComponent();
            PreviewMesh->AttachToComponent(SplineComponent, FAttachmentTransformRules::KeepRelativeTransform);

            //Move to point
            const FTransform PointTransform = SplineComponent->GetTransformAtSplinePoint(i, ESplineCoordinateSpace::World);
            PreviewMesh->SetWorldTransform(PointTransform);

            PreviewMesh->SetStaticMesh(MeshComponent->GetStaticMesh());
            PreviewMesh->SetMaterial(0, PreviewMaterial);
            PreviewMeshComponents.Add(PreviewMesh);
        }
#endif
    }
}

void AMultiplePlaceMesh::BeginPlay()
{
    //Destroy Previews & Spline
    ClearPreviews();
    SplineComponent->DestroyComponent();

    //Move Mesh to a random point
    const int SelectedPoint = FMath::RandRange(0, SplineComponent->GetNumberOfSplinePoints());
    const FTransform PointTransform = SplineComponent->GetTransformAtSplinePoint(SelectedPoint, ESplineCoordinateSpace::World);
    MeshComponent->SetWorldTransform(PointTransform);
    MeshComponent->SetVisibility(true);
}

void AMultiplePlaceMesh::ClearPreviews()
{
    for (auto* MeshC : PreviewMeshComponents) {
        MeshC->DestroyComponent();
    }
    PreviewMeshComponents.Empty();
}

FRandomStream AMultiplePlaceMesh::GetSeed_Implementation()
{
    return FRandomStream();
}

#undef LOCTEXT_NAMESPACE 