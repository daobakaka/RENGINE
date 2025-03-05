#include <iostream>
#include "Monster.h"
#include <vector>
#include <GL/glew.h>//加载Opengl扩展，
#include <GLFW/glfw3.h>//创建Opengl扩展，以及管理Opengl上下文
#include "Cube.h"
#include "Controller.h"
#include "LifecycleManager.h"
#include "IntergratedScripts.h"
#include <list>  // 包含 list 容器
#include "MeshDataManager.h"
#include "CustomModel.h"
#include "Light.h"
#include "FileLoadIO.h"
#include "TextRender.h"
#include "CoroutineMethod.h"
#include "ScriptModel.h"
#include <chrono>
#include <thread>
#include "ShaderManager.h"
using namespace Game;

//控制组件标识
extern Controller* controller;
extern LifecycleManager<CustomModel>* manager;
extern IntergratedScripts* scripts;
extern MeshDataManager* meshData;
extern TextRender* cusText;
extern CoroutineMethod* coroutine;
extern LightSpawner* lightSpawner;
extern LightRender* lightRender;
extern ShaderManager* shaderManager;

std::vector<CustomModel*> toActiveFalse;
std::vector<CustomModel*> toActiveTrue;
std::vector<CustomModel*> toDestory;
void GameUpdateShadowRenderT()
{


    //--全局执行区域阴影
//使用Controller内部全局着色器
    lightRender->RenderDepthMapForParallelLight(lightSpawner->GetParallelLight().direction);//渲染深度缓冲图，用于阴影，内含绑定_paralleLightDepthMapFBO
    lightRender->BindFramebuffer();
    shaderManager->SetMat4("depthCal", "lightSpaceMatrix", lightRender->GetLightMatrix());
    manager->UpdateDepthPic(lightRender->GetLightMatrix(), shaderManager->GetShader("depthCal"));//获取light的光源矩阵，并为场景中的每个对象绘制进入_paralleLightDepthMapFBO,独立对象的绘制采用模板方法
    lightRender->UnbindFramebuffer();//绘制完成之后，解除绑定阴影渲染的深度贴图
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // 清除颜色缓冲和深度缓冲
    shaderManager->UseShader("depthVisual");
    lightRender->RenderShadowTexture(shaderManager->GetShader("depthVisual"));
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     //--控制平行光的旋转
  //  lightSpawner->ParalletLightController(glm::vec3(0, 0.1F, 0.0f));


}



void GameUpdateMainLogicT(glm::mat4 view,glm::mat4 projection)
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // 清除颜色缓冲和深度缓冲
    //每帧清空可移除对象
    toActiveFalse.clear();
    toActiveTrue.clear();
    toDestory.clear();
    //2.使用综合脚本进行控制，场景类独立性综合性的方法,这个方法也可以通过变体种子int参数来执行不同的脚本
    //遍历执行池
    for (auto &item : manager->GetNativeObjects()) {
        //将多光源照射效果封装在 灯光渲染器中.如果是光照shader，则需要加入这一段代码，引入光照渲染，如果不是则不需要
        //现在更改为使用构造化方式，统一使用
       //--是否光照模型判断
        if (item.second->ifLight)
        {
            //这里既可以使用独有shader 也可以使用共有shader，看使用什么类去构造
            lightRender->RenderLights(item.second->shaderProgram, controller, lightSpawner, item.second->position);//更改渲染逻辑，减少访问次数
        }

        if (item.second->GetVariant() == 0)
        {

            scripts->TChangeRandom(-0.01f, 0.01f);//改变构造随机种子
            // scripts->CubeUpdateFun(item); // 使用迭代器遍历链表并调用每个,暂时理解为一个遍历语法糖
        }
        else if (item.second->GetVariant() == ModelClass::CubeTestE)
        {
            //脚本方法，执行综合方法
          scripts->TestUpdateFun(item.second);
            //  baseSphere->AttachTexture(TextureDic["butterfly"][0], 1);        
        }
        else if (item.second->GetVariant() == ModelClass::ParallelLight)//平行光旋转，后面增加其他逻辑
        {

            scripts->TParallelLightRotation(item.second);

        }
        else if (item.second->GetVariant() == ModelClass::ActorButterfly)
        {
            //目前这种统一脚本的方式，并不能完全独立于对象脚本，只能在一定程度上进行独立
            scripts->ActorButtfly(item.second);
            item.second->PlayAnimation(0, 0.1f);


        }
        else if (item.second->GetVariant() == ModelClass::TsetButterfly)
        {
            item.second->PlayAnimation(0, 0.1f);
           // std::cout << item.second->GetID() << "ID" << item.second->_ifShadow << std::endl;
        }
        else if (item.second->GetVariant() == ModelClass::TestPhysics)
        {
            if(item.second->position.y<-10)
            toActiveFalse .push_back(item.second);
        }
    }

    //遍历缓存池
    for (auto& item : manager->GetCacheObjects())
    {

        if (item.second->GetVariant() == ModelClass::TestPhysics)
        {
            toActiveTrue.push_back(item.second);
        }

    }
    // 临时存取失活对象
    for (auto* obj : toActiveFalse) {
        manager->SetActive(obj,false);
    }
    //临时存取激活对像
    for (auto* obj : toActiveTrue) {
        obj->position += glm::vec3(0, 50, 0);
        obj->GetComponent<PhysicalEngine>()->SetVelocity(glm::vec3(0));
        manager->SetActive(obj, true);
    }
#pragma region 恢复逻辑


#pragma endregion

  
   

}


