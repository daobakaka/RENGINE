#include "CollisionBody.h"
#include <iostream>
#include "PhysicalEngine.h"

namespace Game {


    std::unordered_map<int, CollisionProperties*> CollisionProps;
    int CollisionBody::NEXTINT = 0;
    // 构造函数
    CollisionBody::CollisionBody(glm::vec3& pos, glm::vec3& vel, glm::vec3& acc, glm::quat& rot, float mass, float friction, float elasticity,bool ifStatic=false)
        :_collisionProperties(pos, vel, acc,rot)//初始化结构体引用
    {
        
        _collisionProperties.mass = mass;
        _collisionProperties.friction = friction;
        _collisionProperties.ID = NEXTINT++;//现在使用碰撞结构体储存相关的参数包括ID，碰撞检测中也直接与结构体进行快速交互
        _collisionProperties.staticObj = ifStatic;
        _collisionProperties.gravityEnabled = true;//默认均开启重力，重力可以参数初始加速度来影响重力，这里的开启关闭只是用于重力的储存计算
        if (elasticity>1)
        {
            elasticity = 1;
        }
        _collisionProperties.elasticity = elasticity;
        if (ifStatic)
            _collisionProperties.mass = 100000000;//设置静态物体的质量为亿        
    }
    // 析构函数
    CollisionBody::~CollisionBody() {}

    bool CollisionBody::Interface()
    {
        return false;
    }

    // 更新物体的速度和位置
    void CollisionBody::UpdateCollisionState(std::unordered_map<int, CollisionProperties*>& cop,float deltaTime) {
        
        
        // 清空碰撞容器
        _collisionProperties.collidingBodies.clear();
        
        UpdateCollisionParameters();
        //先采用暴力碰撞进行测试
        for (auto& pair : cop)
        {
            if (_collisionProperties.ID != pair.first)
            {
                //这里可以根据碰撞层来进行碰撞分类

                CheckCollisionWithAABB(pair.second);//传入引用，这里才需要使用*访问值，而传入指针就不需要
            }
        }
        //更新碰撞状态
        _collisionProperties.isCollision = !_collisionProperties.collidingBodies.empty();
        //碰撞体的后续物理计算，放到碰撞体类
        UpdatePhysicsCollision();
      
        // 在这里打印碰撞体的参数
       // std::cout << "Collision Body ID: " << _collisionProperties.ID << "\n";
        //std::cout << "Position: (" << _collisionProperties.position.x << ", "
        //    << _collisionProperties.position.y << ", "
        //    << _collisionProperties.position.z << ")\n";

        //std::cout << "Velocity: (" << _collisionProperties.velocity.x << ", "
        //    << _collisionProperties.velocity.y << ", "
        //    << _collisionProperties.velocity.z << ")\n";

        //std::cout << "Acceleration: (" << _collisionProperties.acceleration.x << ", "
        //    << _collisionProperties.acceleration.y << ", "
        //    << _collisionProperties.acceleration.z << ")\n";

        //std::cout << "Mass: " << _collisionProperties.mass << "\n";
        //std::cout << "Friction: " << _collisionProperties.friction << "\n";
        //std::cout << "Collision Min: (" << _collisionProperties._collisionMin.x << ", "
        //    << _collisionProperties._collisionMin.y << ", "
        //    << _collisionProperties._collisionMin.z << ")\n";
        //std::cout << "Collision Max: (" << _collisionProperties._collisionMax.x << ", "
        //    << _collisionProperties._collisionMax.y << ", "
        //    << _collisionProperties._collisionMax.z << ")\n";

        // 打印角速度
        //std::cout << "Angular Velocity: (" << _collisionProperties.angularVelocity.x << ", "
        //    << _collisionProperties.angularVelocity.y << ", "
        //    << _collisionProperties.angularVelocity.z << ")\n";

        //// 打印四元数旋转
        //std::cout << "Rotation (Quaternion): (" << _collisionProperties.rotation.w << ", "
        //    << _collisionProperties.rotation.x << ", "
        //    << _collisionProperties.rotation.y << ", "
        //    << _collisionProperties.rotation.z << ")\n";


    }


    void CollisionBody::SetCollisionParameters(CollisionType type,float radius, glm::vec3 ratio,int layer, bool trigger)
    {

        _collisionProperties.type = type;
        _collisionProperties.radius = radius;
        _collisionProperties.ratio = ratio;
        _collisionProperties.layer = layer;
        _collisionProperties.trigger = trigger;
        //初始化重力加速度，储存在碰撞结构体中，便于计算中引用
        _collisionProperties.gravity = _collisionProperties.acceleration;
       //注入全局访问模块
        RegisterCollisionBody();
    }

    void CollisionBody::RegisterCollisionBody()
    {

        CollisionProps[_collisionProperties.ID] = &_collisionProperties;

    }

    void CollisionBody::UpdateCollisionParameters()
    {
        switch (_collisionProperties.type)
        {
        case CollisionType:: Box:
            _collisionProperties._collisionMin.x = _collisionProperties.position.x - _collisionProperties.radius * _collisionProperties.ratio.x;
            _collisionProperties._collisionMin.y = _collisionProperties.position.y - _collisionProperties.radius * _collisionProperties.ratio.y;
            _collisionProperties._collisionMin.z = _collisionProperties.position.z - _collisionProperties.radius * _collisionProperties.ratio.z;
            _collisionProperties._collisionMax.x = _collisionProperties.position.x + _collisionProperties.radius * _collisionProperties.ratio.x;
            _collisionProperties._collisionMax.y = _collisionProperties.position.y + _collisionProperties.radius * _collisionProperties.ratio.y;
            _collisionProperties._collisionMax.z = _collisionProperties.position.z + _collisionProperties.radius * _collisionProperties.ratio.z;

            break;
        case CollisionType::Sphere:
            _collisionProperties._collisionMin.x = _collisionProperties.position.x - _collisionProperties.radius * _collisionProperties.ratio.x;
            _collisionProperties._collisionMin.y = _collisionProperties.position.y - _collisionProperties.radius * _collisionProperties.ratio.y;
            _collisionProperties._collisionMin.z = _collisionProperties.position.z - _collisionProperties.radius * _collisionProperties.ratio.z;
            _collisionProperties._collisionMax.x = _collisionProperties.position.x + _collisionProperties.radius * _collisionProperties.ratio.x;
            _collisionProperties._collisionMax.y = _collisionProperties.position.y + _collisionProperties.radius * _collisionProperties.ratio.y;
            _collisionProperties._collisionMax.z = _collisionProperties.position.z + _collisionProperties.radius * _collisionProperties.ratio.z;

            break;
        default:
            break;
        }


    }

    void CollisionBody::ResolveCollision(CollisionProperties* other) {
        
      //判断碰撞体不为触发器，且自身碰撞结构不为静态
        if (!other->trigger&&!_collisionProperties.staticObj) {
          //  std::cout << "外部计算碰撞" <<std::endl;
            //计算碰撞法向量
            auto collisionNormal = CalculateCollisionNormal(other);
            //更新物理属性中的它摩擦系数值，便于后期计算
            _collisionProperties.otherFriction = other->friction;
            // 计算两物体的相对速度（沿着碰撞法向量的速度）
            float relativeVelocity = glm::dot(_collisionProperties.velocity - other->velocity, collisionNormal);


            // 如果相对速度大于0，则发生碰撞
            if (relativeVelocity >1.05- _collisionProperties.elasticity) {
               
                std::cout << _collisionProperties.ID<<"内部计算碰撞" << std::endl;

                // 弹性系数（恢复系数）
                float e = _collisionProperties.elasticity;  //从物理碰撞模块中读取弹性系数

                // 计算冲量
                float impulse = -(1 + e) * relativeVelocity /
                    (1 / _collisionProperties.mass + 1 / other->mass);

                // 更新当前物体的速度
                glm::vec3 newVelocity = _collisionProperties.velocity + impulse * collisionNormal / _collisionProperties.mass;

                // 设置更新后的速度
                _collisionProperties.velocity = newVelocity;



                // 计算碰撞点到质心的向量
                glm::vec3 r = _collisionProperties.position - other->position;

                // 计算扭矩
                glm::vec3 torque = glm::cross(r, impulse * collisionNormal);

                // 临时计算角速度
                _collisionProperties.angularVelocity = torque / _collisionProperties.mass; // 假设转动惯量为质量
              
            }

        }
    }

    glm::vec3 CollisionBody::CalculateCollisionNormal(CollisionProperties* other) {
        // 获取当前物体和对方物体的 AABB 边界
        glm::vec3 currentMin = _collisionProperties._collisionMin;
        glm::vec3 currentMax = _collisionProperties._collisionMax;
        glm::vec3 otherMin = other->_collisionMin;
        glm::vec3 otherMax = other->_collisionMax;

        // 计算碰撞点（假设碰撞点是当前物体的中心）
        glm::vec3 collisionPoint = _collisionProperties.position;

        // 计算碰撞点到对方物体每个面的距离
        float distToLeft = collisionPoint.x - otherMin.x;   // 左面 (x = minX)
        float distToRight = otherMax.x - collisionPoint.x;  // 右面 (x = maxX)
        float distToBottom = collisionPoint.y - otherMin.y; // 下面 (y = minY)
        float distToTop = otherMax.y - collisionPoint.y;    // 上面 (y = maxY)
        float distToBack = collisionPoint.z - otherMin.z;   // 后面 (z = minZ)
        float distToFront = otherMax.z - collisionPoint.z;  // 前面 (z = maxZ)

        // 计算每个面的权重（距离越小，权重越大）
        float weightLeft = 1.0f / (distToLeft + 1e-5f);   // 避免除零
        float weightRight = 1.0f / (distToRight + 1e-5f);
        float weightBottom = 1.0f / (distToBottom + 1e-5f);
        float weightTop = 1.0f / (distToTop + 1e-5f);
        float weightBack = 1.0f / (distToBack + 1e-5f);
        float weightFront = 1.0f / (distToFront + 1e-5f);

        // 计算加权平均的法向量
        glm::vec3 collisionNormal =
            (weightLeft * glm::vec3(-1.0f, 0.0f, 0.0f) +  // 左面法向量
                (weightRight * glm::vec3(1.0f, 0.0f, 0.0f)) + // 右面法向量
                (weightBottom * glm::vec3(0.0f, -1.0f, 0.0f)) + // 下面法向量
                (weightTop * glm::vec3(0.0f, 1.0f, 0.0f)) +   // 上面法向量
                (weightBack * glm::vec3(0.0f, 0.0f, -1.0f)) + // 后面法向量
                (weightFront * glm::vec3(0.0f, 0.0f, 1.0f)));  // 前面法向量

        // 归一化法向量
        collisionNormal = glm::normalize(collisionNormal);

        return collisionNormal;
    }

    void CollisionBody::UpdateGravityState(float normalY)
    {

    }

    void CollisionBody::UpdatePhysicsCollision(float deltaTime)
    {
        //非静态物体才进行物理计算
        if (!_collisionProperties.staticObj)
        {

            // 更新速度：v = v0 + a * t
            _collisionProperties.velocity += _collisionProperties.acceleration * deltaTime;
            //速度矫正
            if (glm::dot(_collisionProperties.velocity, _collisionProperties.velocity) < 0.01f)
            {
                _collisionProperties.velocity = glm::vec3(0.0f);
            }
          
            //增加摩擦系数对物体运动的影响
            if (_collisionProperties.isCollision)
            {
                float totalFriction = (_collisionProperties.friction + _collisionProperties.otherFriction) / 1.5f;
        
                // 计算摩擦加速度
                glm::vec3 frictionAcceleration = totalFriction * _collisionProperties.gravity.y * glm::normalize(glm::vec3(_collisionProperties.velocity.x, 0, _collisionProperties.velocity.z));
                //当水平速度大于某个值时
                if (glm::dot(glm::vec2(_collisionProperties.velocity.x, _collisionProperties.velocity.z), glm::vec2(_collisionProperties.velocity.x, _collisionProperties.velocity.z)) > 0.01f)
                {  // 更新物体的速度
                    _collisionProperties.velocity += frictionAcceleration * deltaTime;
                }
                else
                {
                    _collisionProperties.velocity.x = 0;
                    _collisionProperties.velocity.z = 0;
                }
            }

            // 更新旋转角度（四元数）
            float angularSpeedSquared = glm::dot(_collisionProperties.angularVelocity, _collisionProperties.angularVelocity);
            if (angularSpeedSquared > 0.01f) {
                // 计算旋转角度
                float angle = glm::sqrt(angularSpeedSquared) * deltaTime;

                // 计算旋转轴
                glm::vec3 axis = _collisionProperties.angularVelocity / glm::sqrt(angularSpeedSquared);

                // 计算旋转增量（四元数）
                glm::quat deltaRotation = glm::angleAxis(angle, axis);

                // 更新旋转
                _collisionProperties.rotation = deltaRotation * _collisionProperties.rotation;

            }
            else
            {
                // 重置角速度
                _collisionProperties.angularVelocity = glm::vec3(0.0f);
            }


            // 更新位置：s = s0 + v * t
            _collisionProperties.position += _collisionProperties.velocity * deltaTime;
            
            //防止地板穿模
            if (_collisionProperties.isCollision)
            {
                if (_collisionProperties.velocity.y <0&&_collisionProperties.position.y<=_collisionProperties.ratio.y)
                {
                    _collisionProperties.position.y = _collisionProperties.ratio.y;
                    _collisionProperties.velocity.y = 0;
                }
            }
           // std::cout << _collisionProperties.isCollision << std::endl;

            // 在这里打印碰撞体的参数
            //std::cout << "Position: (" << _collisionProperties.position.x << ", "
            //    << _collisionProperties.position.y << ", "
            //    << _collisionProperties.position.z << ")\n";

            //std::cout << "Velocity: (" << _collisionProperties.velocity.x << ", "
            //    << _collisionProperties.velocity.y << ", "
            //    << _collisionProperties.velocity.z << ")\n";

            //std::cout << "Acceleration: (" << _collisionProperties.acceleration.x << ", "
            //    << _collisionProperties.acceleration.y << ", "
            //    << _collisionProperties.acceleration.z << ")\n";

            //std::cout << "Mass: " << _collisionProperties.mass << "\n";
            //std::cout << "Friction: " << _collisionProperties.friction << "\n";
            //std::cout << "Collision Min: (" << _collisionProperties._collisionMin.x << ", "
            //    << _collisionProperties._collisionMin.y << ", "
            //    << _collisionProperties._collisionMin.z << ")\n";
            //std::cout << "Collision Max: (" << _collisionProperties._collisionMax.x << ", "
            //    << _collisionProperties._collisionMax.y << ", "
            //    << _collisionProperties._collisionMax.z << ")\n";
        }
    }

    

    // 与 AABB 碰撞检测
    bool CollisionBody::CheckCollisionWithAABB(CollisionProperties* other) {
        // 获取当前物体和对方物体的 AABB 边界
        glm::vec3 thisMin = _collisionProperties._collisionMin;
        glm::vec3 thisMax = _collisionProperties._collisionMax;
        glm::vec3 otherMin = other->_collisionMin;
        glm::vec3 otherMax = other->_collisionMax;

        // 检测两个 AABB 是否相交
        bool isColliding = (thisMin.x <= otherMax.x && thisMax.x >= otherMin.x) &&
            (thisMin.y <= otherMax.y && thisMax.y >= otherMin.y) &&
            (thisMin.z <= otherMax.z && thisMax.z >= otherMin.z);

        if (isColliding) {
            // 如果当前碰撞体不在碰撞容器中，则添加并处理碰撞
            if (_collisionProperties.collidingBodies.find(other->ID) == _collisionProperties.collidingBodies.end()) {
               // std::cout << _collisionProperties.ID << " 碰撞了 " << other->ID << std::endl;
                _collisionProperties.collidingBodies.insert(other->ID); // 添加到碰撞容器
                ResolveCollision(other); // 处理碰撞
            }
            return true; // 碰撞发生
        }
        else {
            // 如果当前碰撞体在碰撞容器中，则移除并标记碰撞结束
            if (_collisionProperties.collidingBodies.find(other->ID) != _collisionProperties.collidingBodies.end()) {
                std::cout << _collisionProperties.ID << " 碰撞结束 " << other->ID << std::endl;
                _collisionProperties.collidingBodies.erase(other->ID); // 从碰撞容器移除
            }
            return false; // 碰撞未发生
        }
    }


}
