#ifndef PHYSICAL_ENGINE_H
#define PHYSICAL_ENGINE_H

#include "GameObject.h"
#include <glm/glm.hpp>

namespace Game {


    class CollisionBody;
    class PhysicalEngine : public IComponent<PhysicalEngine> {
    public:
        PhysicalEngine(glm::vec3& positon,glm::quat &rotation,int id,bool staticObj=false);  // 构造函数
        ~PhysicalEngine();

        // 更新物体的物理状态（例如位置、速度、加速度）
        void UpdatePhysics(float deltaTime = 0.0167f);
        // 设置物理组件的基本参数
        void SetMass(float mass);
        void SetFriction(float friction);
        void SetVelocity( glm::vec3 velocity);
        void SetAcceleration( glm::vec3 acceleration);

        void SetParameters( float mass, float friction,  glm::vec3 velocity,  glm::vec3 acceleration, float elasticity);
        

        // 获取当前物体的物理状态const拓宽范围常用于get方法标识
        glm::vec3 &GetPosition() ;
        glm::vec3 &GetVelocity() ;
        glm::vec3 &GetAcceleration() ;
        glm::quat& GetRotation();
        float GetMass() const;
        float GetFriction() const;
        bool GetStatic()const;
        float GetElasticity()const;

            
            
    private:
        bool Interface() override;
        //设置旋转阻尼，默认0.15f
        void SetRotationDamping(float damp=0.15f);
        //设置默认旋转锁定，默认设置锁定X和Z;
        bool SetFixedAxisX(bool lock = true);

        bool GetLockState() const;
        float GetDampping() const;

    private:
        float _mass;                  // 质量
        float _friction;              // 摩擦系数
        glm::vec3 &_position;          // 物体位置,引用类型必须通过构造函数初始化，这样内部就可以直接修改
        glm::vec3 _velocity;          // 物体速度
        glm::vec3 _acceleration;      // 物体加速度
        glm::quat &_rotation;        //物体旋转逻辑
        glm::vec3 _gravity;        //物体重力加速度
        bool _ifStatic;      //是否为静态物体
        float _elasticity;//物体弹性系数
        int ID;
        bool _lockxz;
        float _rotationDampping;

    };

}  // namespace Game

#endif // !PHYSICAL_ENGINE_H
