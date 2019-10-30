#pragma once

#include <map>
#include <string>
#include <list>
#include <vector>
#include <algorithm>
#include <stdexcept>

#pragma pack(1)

#ifdef __cplusplus
extern "C" {
#endif

    namespace VRM {
        typedef float(vec3)[3];
        typedef float(vec4)[4];

        struct ASSIMP_API VRMBlendShapeBind {
            aiString blendShapeName;
            aiString nodeName;
            aiString meshName;
            int meshID=0;
            int shapeIndex=0;
            int weight=100;
        };

        struct ASSIMP_API VRMBlendShapeGroup {
            aiString groupName;

            int bindNum=0;
            VRMBlendShapeBind *bind = nullptr;

            ~VRMBlendShapeGroup() {
                delete[] bind;
            }
        };

        struct ASSIMP_API VRMHumanoid {
            aiString humanBoneName;
            aiString nodeName;
        };

        // physics
        struct ASSIMP_API VRMSpring {
            float stiffiness = 0.f;
            float gravityPower = 0.f;
            vec3 gravityDir = { 0,0,0 };
            float dragForce = 0.f;
            float hitRadius = 0.f;

            int boneNum = 0;
            int *bones;
            aiString *bones_name = nullptr;

            int colliderGourpNum = 0;
            int* colliderGroups = nullptr;

            ~VRMSpring() {
                delete[] bones_name;
                delete[] colliderGroups;
            }
        };
        struct ASSIMP_API VRMCollider {
            vec3 offset = { 0,0,0 };
            float radius = 0.f;
        };
        struct ASSIMP_API VRMColliderGroup {
            int node = 0;
            aiString node_name;

            int colliderNum = 0;
            VRMCollider *colliders = nullptr;

            ~VRMColliderGroup() {
                delete[] colliders;
            }
        };
        // physics

        // material
        struct ASSIMP_API VRMMaterialFloatProperties {
            float _Cutoff;
            float _BumpScale;
            float _ReceiveShadowRate;
            float _ShadeShift;
            float _ShadeToony;
            float _LightColorAttenuation;
            float _IndirectLightIntensity;
            float _RimLightingMix;
            float _RimFresnelPower;
            float _RimLift;
            float _OutlineWidth;
            float _OutlineScaledMaxDistance;
            float _OutlineLightingMix;
            float _UvAnimScrollX;
            float _UvAnimScrollY;
            float _UvAnimRotation;
            float _MToonVersion;
            float _DebugMode;
            float _BlendMode;
            float _OutlineWidthMode;
            float _OutlineColorMode;
            float _CullMode;
            float _OutlineCullMode;
            float _SrcBlend;
            float _DstBlend;
            float _ZWrite;
        };
        struct ASSIMP_API VRMMaterialVectorProperties {
            vec4 _Color;
            vec4 _ShadeColor;
            vec4 _MainTex;
            vec4 _ShadeTexture;
            vec4 _BumpMap;
            vec4 _ReceiveShadowTexture;
            vec4 _ShadingGradeTexture;
            vec4 _RimColor;
            vec4 _RimTexture;
            vec4 _SphereAdd;
            vec4 _EmissionColor;
            vec4 _EmissionMap;
            vec4 _OutlineWidthTexture;
            vec4 _OutlineColor;
            vec4 _UvAnimMaskTexture;
        };
        struct ASSIMP_API VRMMaterialTextureProperties {
            int _MainTex;
            int _ShadeTexture;
            int _BumpMap;
            int _ReceiveShadowTexture;
            int _ShadingGradeTexture;
            int _RimTexture;
            int _SphereAdd;
            int _EmissionMap;
            int _OutlineWidthTexture;
            int _UvAnimMaskTexture;
        };
        struct ASSIMP_API VRMMaterial {
            aiString name;
            aiString shaderName;
            VRMMaterialFloatProperties floatProperties;
            VRMMaterialVectorProperties vectorProperties;
            VRMMaterialTextureProperties textureProperties;
        };
        // material

        // license
        enum VRMLicenseList{
            LIC_version,
            LIC_author,
            LIC_contactInformation,
            LIC_reference,
            LIC_title,
            LIC_texture,
            LIC_allowedUserName,
            LIC_violentUssageName,
            LIC_sexualUssageName,
            LIC_commercialUssageName,
            LIC_otherPermissionUrl,
            LIC_licenseName,
            LIC_otherLicenseUrl,

            LIC_futter,
            LIC_max,
        };

        struct ASSIMP_API VRMLicensePair {
            aiString Key;
            aiString Value;
        };

        struct ASSIMP_API VRMLicense
        {
            int licensePairNum = 0;
            VRMLicensePair *licensePair = nullptr;

            ~VRMLicense() {
                delete[] licensePair;
            }
        };
        // license end

        struct ASSIMP_API VRMMetadata
        {
            VRMLicense license;

            int sprintNum = 0;
            VRMSpring *springs = nullptr;

            int colliderGroupNum = 0;
            VRMColliderGroup *colliderGroups = nullptr;

            VRMHumanoid humanoidBone[55];

            int blensShapeGroupNum = 0;
            VRMBlendShapeGroup *blensShapeGourp = nullptr;

            int materialNum = 0;
            VRMMaterial *material = nullptr;

            VRMMetadata() {
            }

            ~VRMMetadata() {
                delete[] springs;
                delete[] colliderGroups;
                delete[] blensShapeGourp;
                delete[] material;
            }

        };

        void ReleaseVRMMeta(VRMMetadata *&meta);
    }

#ifdef __cplusplus
}
#endif

#pragma pack()

