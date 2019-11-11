/* TypeScript file generated by genType. */
/* eslint-disable import/first */


// tslint:disable-next-line:no-var-requires
const Curry = require('bs-platform/lib/es6/curry.js');

// tslint:disable-next-line:no-var-requires
const ABS = require('./A.bs');

import {RequestInit_t as Fetch_RequestInit_t} from 'bs-fetch/src/Fetch.gen';

import {bodyInit as Fetch_bodyInit} from 'bs-fetch/src/Fetch.gen';

import {headersInit as Fetch_headersInit} from 'bs-fetch/src/Fetch.gen';

import {referrerPolicy as Fetch_referrerPolicy} from 'bs-fetch/src/Fetch.gen';

import {requestCache as Fetch_requestCache} from 'bs-fetch/src/Fetch.gen';

import {requestCredentials as Fetch_requestCredentials} from 'bs-fetch/src/Fetch.gen';

import {requestMethod as Fetch_requestMethod} from 'bs-fetch/src/Fetch.gen';

import {requestMode as Fetch_requestMode} from 'bs-fetch/src/Fetch.gen';

import {requestRedirect as Fetch_requestRedirect} from 'bs-fetch/src/Fetch.gen';

// tslint:disable-next-line:interface-over-type-literal
export type z = Fetch_requestMethod;

export const q: (_1:{
  readonly method_?: Fetch_requestMethod; 
  readonly headers?: Fetch_headersInit; 
  readonly body?: Fetch_bodyInit; 
  readonly referrer?: string; 
  readonly referrerPolicy?: Fetch_referrerPolicy; 
  readonly mode?: Fetch_requestMode; 
  readonly credentials?: Fetch_requestCredentials; 
  readonly cache?: Fetch_requestCache; 
  readonly redirect?: Fetch_requestRedirect; 
  readonly integrity?: string; 
  readonly keepalive?: boolean
}, _2:void) => Fetch_RequestInit_t = function (Arg1: any, Arg2: any) {
  const result = Curry.app(ABS.q, [Arg1.method_, Arg1.headers, Arg1.body, Arg1.referrer, Arg1.referrerPolicy, Arg1.mode, Arg1.credentials, Arg1.cache, Arg1.redirect, Arg1.integrity, Arg1.keepalive, Arg2]);
  return result
};
