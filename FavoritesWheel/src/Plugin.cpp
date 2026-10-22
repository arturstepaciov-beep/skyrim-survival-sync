#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <mutex>
#include <numbers>
#include <string>
#include <vector>
#include "Policy.h"
extern "C" IMAGE_DOS_HEADER __ImageBase;
namespace FW {
constexpr float Pi=std::numbers::pi_v<float>, Tau=2*Pi;
struct Item { RE::FormID id{},uniqueBase{}; std::uint16_t unique{}; int extraIndex{-1}; std::string name,kind; int count{1}; };
struct State {
 std::mutex mutex;
 bool open{},settings{},middle{},captureMiddle{},rendererReady{};
 int mode{},hand{1},category{},page{},radius{250},sensitivity{100};
 float x{},y{},width{1920},height{1080};
 std::array<std::vector<Item>,3> items;
} state;
std::filesystem::path ConfigPath() {
 wchar_t path[MAX_PATH]{}; GetModuleFileNameW(reinterpret_cast<HMODULE>(&__ImageBase),path,MAX_PATH);
 return std::filesystem::path(path).replace_extension(L".ini");
}
void Save() {
 const auto path=ConfigPath().wstring();
 for(auto [key,value]: {std::pair{L"SelectionMode",state.mode},{L"Hand",state.hand},{L"LastWheel",state.category},{L"Radius",state.radius},{L"SensitivityPercent",state.sensitivity}})
  WritePrivateProfileStringW(L"Wheel",key,std::to_wstring(value).c_str(),path.c_str());
}
void Load() {
 const auto path=ConfigPath().wstring();
 auto read=[&](const wchar_t* key,int value){return static_cast<int>(GetPrivateProfileIntW(L"Wheel",key,value,path.c_str()));};
 state.mode=std::clamp(read(L"SelectionMode",0),0,1); state.hand=std::clamp(read(L"Hand",1),0,1);
 state.category=std::clamp(read(L"LastWheel",0),0,2); state.radius=std::clamp(read(L"Radius",250),180,380);
 state.sensitivity=std::clamp(read(L"SensitivityPercent",100),25,250);
}
bool Ready() {
 auto* p=RE::PlayerCharacter::GetSingleton(); auto* ui=RE::UI::GetSingleton();
 if(!p || !p->Get3D() || p->IsDead() || !p->GetParentCell() || !p->GetParentCell()->IsAttached() || !ui || ui->GameIsPaused() || ui->numItemMenus) return false;
 for(auto name:{"Main Menu","Loading Menu","LoadingMenu","Console","Dialogue Menu","FavoritesMenu","MapMenu","TweenMenu","Journal Menu","Sleep/Wait Menu"})
  if(ui->IsMenuOpen(name))return false;
 return true;
}
int Category(RE::TESForm* form) {
 if(form->Is(RE::FormType::Weapon,RE::FormType::Armor,RE::FormType::Ammo,RE::FormType::Light))return 0;
 if(form->Is(RE::FormType::Spell,RE::FormType::Shout,RE::FormType::Scroll))return 1;
 if(form->Is(RE::FormType::AlchemyItem))return 2;
 return -1;
}
void Snapshot() {
 for(auto& list:state.items)list.clear();
 auto* p=RE::PlayerCharacter::GetSingleton(); if(!p)return;
 auto inventory=p->GetInventory();
 for(auto& [object,stack]:inventory) {
  auto& [count,entry]=stack; if(!object || count<=0 || !entry || !entry->IsFavorited())continue;
  int category=Category(object); if(category<0)continue;
  int index=0;
  if(entry->extraLists)for(auto* extra:*entry->extraLists) {
   const int current=index++; if(!extra || !extra->HasType<RE::ExtraHotkey>())continue;
   Item item; item.id=object->GetFormID(); item.extraIndex=current;
   auto* unique=extra->GetByType<RE::ExtraUniqueID>(); if(unique){item.unique=unique->uniqueID;item.uniqueBase=unique->baseID;}
   const char* name=extra->GetDisplayName(object); item.name=name&&*name?name:object->GetName();
   item.count=std::min(count,std::max(1,extra->GetCount()));
   item.kind=category==0?(object->Is(RE::FormType::Armor)?"ARMOR":"GEAR"):category==1?"SCROLL":"POTION";
   if(auto* potion=object->As<RE::AlchemyItem>();potion&&potion->IsPoison())item.kind="POISON";
   state.items[category].push_back(std::move(item));
  }
 }
 if(auto* favorites=RE::MagicFavorites::GetSingleton())for(auto* form:favorites->spells) {
  if(!form || Category(form)!=1)continue;
  Item item;item.id=form->GetFormID();item.name=form->GetName(); item.kind=form->Is(RE::FormType::Shout)?"SHOUT":"SPELL";
  if(auto* spell=form->As<RE::SpellItem>();spell && (spell->GetSpellType()==RE::MagicSystem::SpellType::kPower || spell->GetSpellType()==RE::MagicSystem::SpellType::kLesserPower))item.kind="POWER";
  state.items[1].push_back(std::move(item));
 }
 for(auto& list:state.items)std::stable_sort(list.begin(),list.end(),[](const Item&a,const Item&b){return a.name<b.name;});
}
void Equip(Item item,int hand) {
 SKSE::GetTaskInterface()->AddTask([item=std::move(item),hand] {
  if(!Ready())return;
  auto* player=RE::PlayerCharacter::GetSingleton(); auto* manager=RE::ActorEquipManager::GetSingleton();
  auto* form=RE::TESForm::LookupByID(item.id); if(!form || !manager)return;
  auto* defaults=RE::BGSDefaultObjectManager::GetSingleton(); if(!defaults)return;
  auto* slot=defaults->GetObject<RE::BGSEquipSlot>(hand?20:19);
  if(auto* spell=form->As<RE::SpellItem>()) {
   auto* favorites=RE::MagicFavorites::GetSingleton(); if(!favorites || std::find(favorites->spells.begin(),favorites->spells.end(),form)==favorites->spells.end())return;
   if(item.kind=="POWER")slot=defaults->GetObject<RE::BGSEquipSlot>(22);
   manager->EquipSpell(player,spell,slot); return;
  }
  if(auto* shout=form->As<RE::TESShout>()) {manager->EquipShout(player,shout);return;}
  auto inventory=player->GetInventory(); auto* object=form->As<RE::TESBoundObject>(); auto found=inventory.find(object);
  if(found==inventory.end() || found->second.first<=0 || !found->second.second)return;
  auto& entry=found->second.second; RE::ExtraDataList* selected=nullptr; int index=0;
  if(entry->extraLists)for(auto* extra:*entry->extraLists) {
   int current=index++; if(!extra || !extra->HasType<RE::ExtraHotkey>())continue;
   auto* unique=extra->GetByType<RE::ExtraUniqueID>();
   if(item.unique ? unique && unique->uniqueID==item.unique && unique->baseID==item.uniqueBase : current==item.extraIndex) {
    const char* name=extra->GetDisplayName(object);
    if(!item.unique && item.name!=(name&&*name?name:object->GetName()))continue;
    selected=extra;break;
   }
  }
  if(!selected)return; // Never substitute another enchanted/tempered variant.
  if(!object->Is(RE::FormType::Weapon,RE::FormType::Scroll))slot=nullptr;
  manager->EquipObject(player,object,selected,1,slot,true,false,true,false);
 });
}
int VisibleCount() {return std::clamp(static_cast<int>(state.items[state.category].size())-state.page*WheelPolicy::Slots,0,WheelPolicy::Slots);}
float Radius(){return std::min(static_cast<float>(state.radius),std::min(state.width,state.height)*.34f);}
int Selected(){return WheelPolicy::Hit(state.x,state.y,Radius()*.32f,VisibleCount());}
void Close(bool commit) {
 if(commit && !state.settings) {int index=Selected(); if(index>=0)Equip(state.items[state.category][state.page*WheelPolicy::Slots+index],state.hand);}
 state.open=false;state.settings=false;state.x=state.y=0;Save();
}
void Open() {
 Snapshot();state.open=true;state.settings=false;state.page=0;state.x=state.y=0;
 if(auto* player=RE::PlayerCharacter::GetSingleton())player->InterruptCast(false);
}
void ChangeCategory(int delta){state.category=WheelPolicy::Wrap(state.category+delta,3);state.page=0;state.x=state.y=0;Save();}
void Page(int delta){state.page=WheelPolicy::Wrap(state.page+delta,WheelPolicy::Pages(static_cast<int>(state.items[state.category].size())));state.x=state.y=0;}
bool Handle(RE::InputEvent* event) {
 if(!event)return false;
 if(auto* move=event->AsMouseMoveEvent();move&&state.open) {
  state.x+=move->mouseInputX*state.sensitivity/100.f;state.y+=move->mouseInputY*state.sensitivity/100.f;
  float limit=Radius()*1.55f;
  state.x=std::clamp(state.x,-limit,limit);state.y=std::clamp(state.y,-limit,limit);return true;
 }
 auto* button=event->AsButtonEvent();if(!button)return false;
 const auto id=button->GetIDCode();const bool down=button->IsDown(),up=button->IsUp();
 if(button->GetDevice()==RE::INPUT_DEVICE::kMouse && id==2) {
  if(down && state.rendererReady && Ready()) {state.middle=true;state.captureMiddle=true;if(!state.open)Open();else if(state.mode==1)Close(false);}
  if(up) {state.middle=false;if(state.open && state.mode==0 && !state.settings)Close(true);bool consume=state.captureMiddle;state.captureMiddle=false;return consume;}
  return state.captureMiddle;
 }
 if(!state.open)return false;
 if(button->GetDevice()==RE::INPUT_DEVICE::kKeyboard) {
  if(id==1 || id==15) {if(down) {if(state.settings){state.settings=false;if(state.mode==0&&!state.middle)Close(false);}else Close(false);}return true;}
  if(id==59) {if(down){state.settings=!state.settings;if(!state.settings && state.mode==0&&!state.middle)Close(false);}return true;}
  if(state.settings) {
   if(down && (id==2||id==3)){state.mode=id==2?0:1;Save();}
   if(down && id==35){state.hand=1-state.hand;Save();}
   return true;
  }
  if(id==16||id==18){if(down)ChangeCategory(id==16?-1:1);return true;}
  if(id==201||id==209){if(down)Page(id==201?-1:1);return true;}
  return false; // Movement remains available; no time or pause manipulation.
 }
 if(button->GetDevice()==RE::INPUT_DEVICE::kMouse) {
  if((id==8||id==9)&&button->IsPressed()&&!state.settings){Page(id==8?-1:1);return true;}
  if(id==0 && down) {
   if(state.settings) {
    if(std::abs(state.x)<190) {
     if(state.y>-40&&state.y<8)state.mode=0;
     else if(state.y>18&&state.y<66)state.mode=1;
     else if(state.y>76&&state.y<124)state.hand=1-state.hand;
     Save();
    }
   } else if(state.mode==1)Close(true);
  }
  return !up; // Forward button-up so an earlier physical attack cannot get stuck.
 }
 return false;
}
using Dispatch=void(RE::BSTEventSource<RE::InputEvent*>*,RE::InputEvent**);
REL::Relocation<Dispatch> originalInput;
void Input(RE::BSTEventSource<RE::InputEvent*>* source,RE::InputEvent** events) {
 if(!events){originalInput(source,events);return;}
 std::vector<std::pair<RE::InputEvent*,RE::InputEvent*>> links;
 RE::InputEvent* filtered=nullptr;RE::InputEvent** tail=&filtered;
 {
  std::lock_guard lock(state.mutex);
  if(state.open&&!Ready())Close(false);
  for(auto* event=*events;event;event=event->next) {
   links.emplace_back(event,event->next);
   if(!Handle(event)){*tail=event;tail=&event->next;}
  }
  *tail=nullptr;
 }
 originalInput(source,&filtered);
 for(auto& [event,next]:links)event->next=next;
}
using PresentFn=HRESULT(STDMETHODCALLTYPE*)(IDXGISwapChain*,UINT,UINT);
PresentFn originalPresent{};
ID3D11Device* device{};ID3D11DeviceContext* context{};HWND window{};bool initialized{};
ImVec2 Point(float angle,float radius,ImVec2 center){return {center.x+std::cos(angle)*radius,center.y+std::sin(angle)*radius};}
void Text(ImDrawList* draw,ImVec2 pos,const char* text,float size,ImU32 color,float wrap=0) {
 draw->AddText(ImGui::GetFont(),size,pos,color,text,nullptr,wrap);
}
void Draw() {
 std::lock_guard lock(state.mutex);
 if(!state.open)return;
 if(GetForegroundWindow()!=window){state.open=false;state.settings=false;return;}
 auto& io=ImGui::GetIO();state.width=io.DisplaySize.x;state.height=io.DisplaySize.y;
 auto* d=ImGui::GetBackgroundDrawList();ImVec2 c{state.width*.5f,state.height*.5f};float r=Radius();
 const ImU32 white=IM_COL32(238,232,214,255),muted=IM_COL32(168,178,188,255),gold=IM_COL32(232,185,101,255);
 d->AddRectFilled({0,0},io.DisplaySize,IM_COL32(7,13,20,110));
 if(state.settings) {
  d->AddRectFilled({c.x-235,c.y-155},{c.x+235,c.y+190},IM_COL32(17,25,35,248),18);
  Text(d,{c.x-180,c.y-125},"FAVORITES WHEEL / SETTINGS",22,gold);
  const char* labels[]{"1   Hold, point and release","2   Toggle, then click an item",state.hand?"H   Equip hand: RIGHT":"H   Equip hand: LEFT"};
  for(int i=0;i<3;++i){float y=c.y-40+i*58;bool hover=std::abs(state.x)<190&&state.y>y-c.y&&state.y<y-c.y+48;
   d->AddRectFilled({c.x-195,y},{c.x+195,y+48},hover?IM_COL32(82,66,40,255):IM_COL32(31,44,57,255),7);
   Text(d,{c.x-180,y+13},labels[i],18,(i<2&&state.mode==i)?gold:white);
  }
  Text(d,{c.x-180,c.y+150},"Click an option  |  F1 / Esc: back",16,muted);
 } else {
  const char* categories[]{"GEAR","Magic","Potions"};
  Text(d,{c.x-170,c.y-r-98},"Q  <",18,muted);Text(d,{c.x+130,c.y-r-98},">  E",18,muted);
  for(int i=0;i<3;++i){float x=c.x-105+i*95;Text(d,{x,c.y-r-98},categories[i],18,i==state.category?gold:muted);}
  int n=VisibleCount(),selected=Selected();
  for(int i=0;i<n;++i){float a=Tau*i/n-Pi/2,half=Pi/n-.025f;
   d->PathClear();for(int k=0;k<=20;++k)d->PathLineTo(Point(a-half+(2*half*k/20),r+38,c));
   for(int k=20;k>=0;--k)d->PathLineTo(Point(a-half+(2*half*k/20),r*.39f,c));
   // Annular sectors are concave: draw them as paired triangles, not a convex fill.
   d->PathClear();
   for(int k=0;k<20;++k){float aa=a-half+2*half*k/20,bb=a-half+2*half*(k+1)/20;
    ImVec2 p0=Point(aa,r*.39f,c),p1=Point(aa,r+38,c),p2=Point(bb,r+38,c),p3=Point(bb,r*.39f,c);
    auto color=i==selected?IM_COL32(129,95,43,235):IM_COL32(20,32,44,235);
    d->AddTriangleFilled(p0,p1,p2,color);d->AddTriangleFilled(p0,p2,p3,color);
   }
   auto pos=Point(a,r*.72f,c);const auto& item=state.items[state.category][state.page*WheelPolicy::Slots+i];
   float textWidth=n>7?104.f:142.f;
   Text(d,{pos.x-textWidth/2,pos.y-30},item.kind.c_str(),12,gold);
   Text(d,{pos.x-textWidth/2,pos.y-9},item.name.c_str(),16,white,textWidth);
   if(item.count>1){auto count="x"+std::to_string(item.count);Text(d,{pos.x-textWidth/2,pos.y+39},count.c_str(),13,muted);}
  }
  d->AddCircleFilled(c,r*.32f,IM_COL32(11,18,27,248),64);d->AddCircle(c,r*.34f,gold,64,1.5f);
  const char* center=n==0?"No favorites here":selected<0?"Point to choose":state.items[state.category][state.page*WheelPolicy::Slots+selected].name.c_str();
  Text(d,{c.x-r*.27f,c.y-24},center,18,white,r*.54f);
  Text(d,{c.x-r*.23f,c.y+48},state.hand?"RIGHT HAND":"LEFT HAND",12,gold);
  std::string pages=std::to_string(state.page+1)+" / "+std::to_string(WheelPolicy::Pages(static_cast<int>(state.items[state.category].size())));
  Text(d,{c.x-20,c.y+r+56},pages.c_str(),17,gold);
  Text(d,{c.x-245,c.y+r+88},state.mode?"Click: select   MMB / Esc: cancel   F1: settings":"Release MMB: select   Center: cancel   F1: settings",17,white);
  Text(d,{c.x-190,c.y+r+113},"Q / E: category   Mouse wheel: more items",15,muted);
 }
 ImVec2 cursor{c.x+state.x,c.y+state.y};d->AddCircleFilled(cursor,5,gold);d->AddCircle(cursor,9,white,24,1);
}
HRESULT STDMETHODCALLTYPE Present(IDXGISwapChain* chain,UINT sync,UINT flags) {
 if(!initialized) {
  if(SUCCEEDED(chain->GetDevice(__uuidof(ID3D11Device),reinterpret_cast<void**>(&device)))) {
   device->GetImmediateContext(&context);DXGI_SWAP_CHAIN_DESC desc{};chain->GetDesc(&desc);window=desc.OutputWindow;
   ImGui::CreateContext();auto& io=ImGui::GetIO();io.IniFilename=nullptr;io.LogFilename=nullptr;
   io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeui.ttf",19);if(io.Fonts->Fonts.empty())io.Fonts->AddFontDefault();
   initialized=ImGui_ImplDX11_Init(device,context);{std::lock_guard lock(state.mutex);state.rendererReady=initialized;}
  }
 }
 if(initialized) {
  {std::lock_guard lock(state.mutex);if(!state.open)return originalPresent(chain,sync,flags);}
  DXGI_SWAP_CHAIN_DESC desc{};chain->GetDesc(&desc);RECT client{};GetClientRect(window,&client);
  auto& io=ImGui::GetIO();io.DisplaySize={static_cast<float>(client.right),static_cast<float>(client.bottom)};
  static auto previous=std::chrono::steady_clock::now();auto now=std::chrono::steady_clock::now();io.DeltaTime=std::max(.001f,std::chrono::duration<float>(now-previous).count());previous=now;
  ImGui_ImplDX11_NewFrame();ImGui::NewFrame();Draw();ImGui::Render();
  ID3D11Texture2D* back{};ID3D11RenderTargetView* target{};
  if(SUCCEEDED(chain->GetBuffer(0,__uuidof(ID3D11Texture2D),reinterpret_cast<void**>(&back)))) {
   if(SUCCEEDED(device->CreateRenderTargetView(back,nullptr,&target))) {
    ID3D11RenderTargetView* old{};ID3D11DepthStencilView* depth{};context->OMGetRenderTargets(1,&old,&depth);
    context->OMSetRenderTargets(1,&target,nullptr);ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    context->OMSetRenderTargets(1,&old,depth);if(old)old->Release();if(depth)depth->Release();target->Release();
   }back->Release();
  }
 }
 return originalPresent(chain,sync,flags);
}
void Install() {
 auto* data=RE::BSGraphics::Renderer::GetRendererData();if(!data){spdlog::error("Renderer unavailable");return;}
 IDXGISwapChain* chain{};for(auto& w:data->renderWindows)if(w.swapChain){chain=reinterpret_cast<IDXGISwapChain*>(w.swapChain);break;}
 if(!chain){spdlog::error("Swap chain unavailable");return;}
 REL::Relocation<std::uintptr_t> input{REL::RelocationID(67315,68617)};
 auto call=input.address()+0x7B;
 if(*reinterpret_cast<std::uint8_t*>(call)!=0xE8){spdlog::error("Unsupported input layout; wheel left disabled");return;}
 auto** table=*reinterpret_cast<void***>(chain);DWORD old{};
 if(!VirtualProtect(&table[8],sizeof(void*),PAGE_EXECUTE_READWRITE,&old)){spdlog::error("Cannot install Present hook");return;}
 originalPresent=reinterpret_cast<PresentFn>(table[8]);table[8]=reinterpret_cast<void*>(Present);DWORD ignored{};VirtualProtect(&table[8],sizeof(void*),old,&ignored);
 SKSE::AllocTrampoline(32);originalInput=SKSE::GetTrampoline().write_call<5>(call,Input);
 spdlog::info("Favorites Wheel initialized; mode={}, category={}",state.mode,state.category);
}
}
extern "C" __declspec(dllexport) constinit SKSE::PluginVersionData SKSEPlugin_Version=[] {
 SKSE::PluginVersionData v;v.PluginVersion({1,0,0,0});v.PluginName("FavoritesWheel");v.AuthorName("Artur / Codex");v.UsesAddressLibrary();v.UsesStructsPost629();return v;
}();
extern "C" __declspec(dllexport) bool SKSEPlugin_Query(const SKSE::QueryInterface* skse,SKSE::PluginInfo* info) {
 info->infoVersion=SKSE::PluginInfo::kVersion;info->name="FavoritesWheel";info->version=1;
 return !skse->IsEditor() && skse->RuntimeVersion()>=REL::Version{1,5,97,0};
}
extern "C" __declspec(dllexport) bool SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
 SKSE::Init(skse);
 if(auto path=SKSE::log::log_directory()) {
  auto sink=std::make_shared<spdlog::sinks::basic_file_sink_mt>((*path/"FavoritesWheel.log").string(),true);
  spdlog::set_default_logger(std::make_shared<spdlog::logger>("FavoritesWheel",sink));spdlog::flush_on(spdlog::level::info);
 }
 FW::Load();SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* m){
  if(m->type==SKSE::MessagingInterface::kDataLoaded)FW::Install();
  if(m->type==SKSE::MessagingInterface::kPreLoadGame || m->type==SKSE::MessagingInterface::kNewGame){std::lock_guard lock(FW::state.mutex);FW::state.open=false;FW::state.captureMiddle=false;FW::state.middle=false;}
 });return true;
}
