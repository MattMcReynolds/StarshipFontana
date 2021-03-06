#include "SFApp.h"

SFApp::SFApp(std::shared_ptr<SFWindow> window) : fire(0), is_running(true), sf_window(window) {
  int canvas_w, canvas_h;
  SDL_GetRendererOutputSize(sf_window->getRenderer(), &canvas_w, &canvas_h);

  app_box = make_shared<SFBoundingBox>(Vector2(canvas_w, canvas_h), canvas_w, canvas_h);
  player  = make_shared<SFAsset>(SFASSET_PLAYER, sf_window);
  auto player_pos = Point2(canvas_w/2, 50);
  player->SetPosition(player_pos);

  const int number_of_aliens = 5;
  for(int i=0; i<number_of_aliens; i++) {
    // place an alien at width/number_of_aliens * i
    auto alien = make_shared<SFAsset>(SFASSET_ALIEN, sf_window);
    auto pos   = Point2(((canvas_w/number_of_aliens) * i)+60.0f, 550.0f);
    alien->SetPosition(pos);
    aliens.push_back(alien);
  }

  auto coin = make_shared<SFAsset>(SFASSET_COIN, sf_window);
  auto pos  = Point2(canvas_w/2, 400);
  coin->SetPosition(pos);
  coins.push_back(coin);

auto bar = make_shared<SFAsset>(SFASSET_BAR, sf_window);
  pos  = Point2((canvas_w/2), (canvas_h-10));
  bar->SetPosition(pos);
  bars.push_back(bar);

auto base = make_shared<SFAsset>(SFASSET_BASE, sf_window);
  pos  = Point2((canvas_w/2), 10);
  base->SetPosition(pos);
  bases.push_back(base);
}

SFApp::~SFApp() {
}

/**
 * Handle all events that come from SDL.
 * These are timer or keyboard events.
 */
void SFApp::OnEvent(SFEvent& event) {
  SFEVENT the_event = event.GetCode();
  switch (the_event) {
  case SFEVENT_QUIT:
    is_running = false;
    break;
  case SFEVENT_UPDATE:
    OnUpdateWorld();
    OnRender();
    break;
  }
}

int SFApp::OnExecute() {
  // Execute the app
  SDL_Event event;
  while (SDL_WaitEvent(&event) && is_running) {
    // wrap an SDL_Event with our SFEvent
    SFEvent sfevent((const SDL_Event) event);
    // handle our SFEvent
    OnEvent(sfevent);
  }
}

void SFApp::OnUpdateWorld() {
  //Receive keyboard inputs simultaneously
  const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);
  if(keyboardState[SDL_SCANCODE_DOWN]) {
    player->GoSouth();
  }
  if(keyboardState[SDL_SCANCODE_UP]) {
    player->GoNorth();
  }
  if(keyboardState[SDL_SCANCODE_LEFT]) {
    player->GoWest();
  }
  if(keyboardState[SDL_SCANCODE_RIGHT]) {
    player->GoEast();
  }
  if(keyboardState[SDL_SCANCODE_SPACE]) {
    FireProjectile();
  }
  // Update projectile positions
  for(auto p: projectiles) {
    p->BulletNorth();
  }

  for(auto c: coins) {
    c->WiggleTime();
  }

  // Update enemy positions
  for(auto a : aliens) {
    a->GoSouthSlow();
  }

  // Detect collisions
  for(auto p : projectiles) {
    for(auto a : aliens) {
      if(p->CollidesWith(a)) {
        a->HandleCollision();
	p->HandleCollision();
	cout << "Enemy down." << endl;// enemy planes destroy bullets
      }
    }
  }
  for(auto p : projectiles) {
    for(auto c : coins) {
      if(p->CollidesWith(c)) {
        c->HandleCollision();// pigeons don't destroy bullets
      }
    }
  }
  for(auto p : projectiles) {
    for(auto b : bars) {
      if(p->CollidesWith(b)) {
        p->HandleCollision();
      }
    }
  }
  for(auto a : aliens) {
    for(auto s : bases) {
      if(a->CollidesWith(s)) {
        cout << "An enemy plane reached your base! You lose." << endl;
	a->HandleCollision();
      }
    }
  }
    for(auto a : aliens) {
      if(player->CollidesWith(a)) {
	a->HandleCollision();
        cout << "Enemy crashed into you! You lose." << endl;
      }
  }
  
  // remove dead aliens (the long way)
  list<shared_ptr<SFAsset>> tmp;
  for(auto a : aliens) {
    if(a->IsAlive()) {
      tmp.push_back(a);
    }
  }
  aliens.clear();
  aliens = list<shared_ptr<SFAsset>>(tmp);
  // remove dead coins (pigeon)
  list<shared_ptr<SFAsset>> tmp2;
  for(auto c : coins) {
    if(c->IsAlive()) {
      tmp2.push_back(c);
    }
  }
  coins.clear();
  coins = list<shared_ptr<SFAsset>>(tmp2);
  // remove destroyed bullets
  list<shared_ptr<SFAsset>> tmp3;
  for(auto p : projectiles) {
    if(p->IsAlive()) {
      tmp3.push_back(p);
    }
  }
  projectiles.clear();
  projectiles = list<shared_ptr<SFAsset>>(tmp3);
}

void SFApp::OnRender() {
  SDL_RenderClear(sf_window->getRenderer());

  // draw the player
  player->OnRender();
  
  for(auto b: bars) {
    if(b->IsAlive()) {b->OnRender();}
  }

  for(auto b: bases) {
    if(b->IsAlive()) {b->OnRender();}
  }

  for(auto p: projectiles) {
    if(p->IsAlive()) {p->OnRender();}
  }

  for(auto a: aliens) {
    if(a->IsAlive()) {a->OnRender();}
  }

  for(auto c: coins) {
    if(c->IsAlive()) {c->OnRender();}
  }

  // Switch the off-screen buffer to be on-screen
  SDL_RenderPresent(sf_window->getRenderer());
}

void SFApp::FireProjectile() {
  auto pb = make_shared<SFAsset>(SFASSET_PROJECTILE, sf_window);
  auto v  = player->GetPosition();
  pb->SetPosition(v);
  projectiles.push_back(pb);
}
