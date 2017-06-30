window.eval = global.eval = function() {
  throw new Error("window.eval() is disabled for security");
}

import Vue from 'vue';
import _ from 'lodash';
import URI from 'urijs';

import store from './store/index';
import { ObsApiService } from './services/obs-api';
import { WindowService } from './services/window';
import { ConfigFileService } from './services/config-file';
import { HotkeysService } from './services/hotkeys.ts';
import { OnboardingService } from './services/onboarding.ts';

const { ipcRenderer, remote } = window.require('electron');

require('./app.less');

document.addEventListener('DOMContentLoaded', () => {
  const windowService = WindowService.instance;
  const obsApiService = ObsApiService.instance;
  const configFileService = ConfigFileService.instance;
  const query = URI.parseQuery(URI.parse(window.location.href).query);
  const isChild = query.child;

  if (isChild) {
    windowService.setWindowAsChild();
    windowService.setWindowOptions(_.omit(query, ['child']));

    ipcRenderer.on('closeWindow', () => {
      windowService.closeWindow();
    });
  } else {
    configFileService.load();

    ipcRenderer.on('shutdown', () => {
      configFileService.rawSave();
      remote.getCurrentWindow().close();
    });

    windowService.setWindowOptions({ component: 'Main' });

    HotkeysService.instance.bindAllHotkeys();
    OnboardingService.instance;
  }

  window.obs = obsApiService.nodeObs;

  const vm = new Vue({
    el: '#app',
    store,
    render: h => {
      const componentName = windowService.state.options.component;

      return h(windowService.components[componentName]);
    }
  });

  // Used for replacing the contents of this window with
  // a new top level component
  ipcRenderer.on('window-setContents', (event, options) => {
    windowService.setWindowOptions(options);

    // This is purely for developer convencience.  Changing the URL
    // to match the current contents, as well as pulling the options
    // from the URL, allows child windows to be refreshed without
    // losing their contents.
    let newOptions = Object.assign({ child: isChild }, options);
    let newURL = URI(window.location.href).query(newOptions).toString()

    window.history.replaceState({}, '', newURL);
  });
});