import { CoreState, Dolphin } from './Dolphin';
import * as path from 'path'

let dol = new Dolphin({
  orgName: 'ModLoader64',
  orgDomain: 'https://modloader64.com/',
  appName: 'modloader64-dolphin-emu',
  appDisplayName: 'ModLoader64',
  baseDir: process.cwd(),
  userDir: path.join(process.cwd(), 'UserData')
});

dol.onStateChanged = (newState: CoreState) => {
  console.log('New state: ' + CoreState[newState]);
};

dol.onTick = () => {};

dol.onImGui = () => {
  if (dol.button('osd message'))
    dol.displayMessage('hello world', 3000);
};

dol.start({
  path: '../../blast.wad',
  isNandTitle: false,
  savestatePath: undefined
});
