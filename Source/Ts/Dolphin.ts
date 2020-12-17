import * as path from 'path'

interface CreateInfo {
  orgName: string,
  orgDomain: string,
  appName: string,
  appDisplayName: string,
  baseDir: string,
  userDir: string
}

interface BootInfo {
  path: string,
  isNandTitle: boolean,
  savestatePath?: string
}

enum CoreState {
  Uninitialized, Paused, Running, Stopping, Starting
};

class Dolphin {
  private module: any;
  private frontend: any;
  private createInfo?: CreateInfo;
  private eventHandler?: NodeJS.Timeout = undefined;
  private mtCbHandler?: NodeJS.Timeout = undefined;
  private numTicks = 0;
  private coreState = -1;

  public onTick?: () => void = undefined;
  public onImGui?: () => void = undefined;
  public onStateChanged?: (newState: CoreState) => void = undefined;

  constructor(createInfo: CreateInfo) {
    this.module = require(path.join(createInfo.baseDir, 'dolphin.node'));
    this.frontend = new this.module.Frontend();
    this.createInfo = createInfo;
  }

  start(bootInfo: BootInfo) {
    this.frontend.initialize(this.createInfo);
    delete this.createInfo;
    this.eventHandler = setInterval((() => this.frontend.processEvents(4)).bind(this), 16);
    this.mtCbHandler = setInterval(this.handleMtCb.bind(this), 0);
    this.bindCallbacks();
    this.frontend.enableMtCallbacks();
    this.frontend.startup(bootInfo);
    this.handleStateChanged(0);
  }

  private bindCallbacks() {
    this.frontend.on('state-changed', this.handleStateChanged.bind(this));
    this.frontend.on('stop-requested', this.handleStopRequested.bind(this));
    this.frontend.on('stop-complete', this.handleStopComplete.bind(this));
    this.frontend.on('exit-requested', this.handleExitRequested.bind(this));
  }

  private handleStateChanged(newState: CoreState) {
    if (this.coreState != newState) {
      try { if (this.onStateChanged) this.onStateChanged(newState); }
      finally { this.coreState = newState; }
    }
  }

  private handleStopRequested(): boolean {
    if (this.coreState == CoreState.Uninitialized) return true;
    if (this.numTicks < 30) return false;
    let v: boolean = this.frontend.requestStop();
    if (v) this.frontend.disableMtCallbacks();
    return v;
  }

  private handleStopComplete() {
    this.numTicks = 0;
  }

  private handleExitRequested() {
    if (this.eventHandler) clearInterval(this.eventHandler);
    this.frontend.shutdown();
    if (this.mtCbHandler) clearInterval(this.mtCbHandler);
  }

  private handleMtCb() {
    if (this.frontend.isOnTickPending()) {
      this.frontend.signalHandlingOnTick();
      ++this.numTicks;
      try { if (this.onTick) this.onTick(); }
      finally { this.frontend.unlockOnTick(); }
    }

    if (this.frontend.isOnImGuiPending()) {
      this.frontend.signalHandlingOnImGui();
      try { if (this.onImGui) this.onImGui(); }
      finally { this.frontend.unlockOnImGui(); }
    }
  }

  displayMessage(text: string, timeMs: number) {
    this.frontend.displayMessage(text, timeMs);
  }

  button(text: string): boolean {
    return this.frontend.button(text);
  }
}

export { CreateInfo, BootInfo, CoreState, Dolphin };
