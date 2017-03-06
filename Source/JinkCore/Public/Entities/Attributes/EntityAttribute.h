// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "AttributeModification.h"

#include "EntityAttribute.generated.h"

class AEntity;

DECLARE_DYNAMIC_DELEGATE_OneParam(FModifiedDelegate, FAttributeModification&, Modification);

/**
 * Entity Attribute
 * Used as a modificable float depending on modifiers.
 * When used in entities, call SetOwner in constructor to apply Buff/Debuff modifications.
 */
USTRUCT(BlueprintType)
struct JINKCORE_API FEntityAttribute
{
    GENERATED_BODY()

    FEntityAttribute() {
        BaseValue = 0;
        Guid = FGuid::NewGuid();
    }

    FEntityAttribute(float _BaseValue) {
        BaseValue = _BaseValue;
        Guid = FGuid::NewGuid();
    }

    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGuid Guid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
    float BaseValue;
	
    UPROPERTY(EditAnywhere, Category = "Attributes")
    TArray<FAttributeModification> Modifications;

    UPROPERTY()
    TArray<FAttributeModification> BuffModifications;

    UPROPERTY()
    FModifiedDelegate OnModified;

    void AddModification(FAttributeModification& Modification) {
        Modifications.Add(Modification);
        OnModified.Execute(Modification);
    }

    void RemoveModification(FAttributeModification& Modification) {
        Modifications.Remove(Modification);
        OnModified.Execute(Modification);
    }

    void AddBuffModification(FAttributeModification& Modification) {
        BuffModifications.Add(Modification);
        OnModified.Execute(Modification);
    }
    void RemoveBuffModification(FAttributeModification& Modification) {
        BuffModifications.Remove(Modification);
        OnModified.Execute(Modification);
    }

    const float GetValue() const;

    /* Assign a base value from float */

    /* Get Attribute final value */
    FORCEINLINE operator float() const
    {
        return GetValue();
    }

    //compare two modifications by guid
    FORCEINLINE bool operator==(const FEntityAttribute& Other) const
    {
        return Guid == Other.Guid;
    }
};
