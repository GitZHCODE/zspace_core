import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
import {fileURLToPath} from 'node:url';
const base = new URL('../out/openusd/', import.meta.url);
const {default:factory} = await import(new URL('zspace_core.js',base));
const m = await factory({wasmBinary:await readFile(new URL('zspace_core.wasm',base)),locateFile:n=>fileURLToPath(new URL(n,base))});
const call=(name,...args)=>m.ccall('USDdoc_'+name,'number',args.map(x=>typeof x==='string'?'string':'number'),args);
const check=(name,...args)=>assert.equal(call(name,...args),1,m.UTF8ToString(call('error')));
try {
 m.FS.writeFile('/tmp/object.usda','#usda 1.0\ndef Scope "Input" {}\n');
 check('open','/tmp/object.usda');
 check('definePrimDirect','/Output','Scope');
 check('createAttribute','/Output','width','float',1);
 check('setNumeric',3,0,0,2.5);
 check('setAttributeDirect','/Output','width');
 check('getAttribute','/Output','width');
 assert.equal(call('scalar'),2.5);
 check('setString','wrong');
 assert.equal(call('setAttributeDirect','/Output','width'),0);
 check('getAttribute','/Output','width'); assert.equal(call('scalar'),2.5);
 check('save','/tmp/output.usda');
 check('open','/tmp/output.usda');
 check('getAttribute','/Output','width'); assert.equal(call('scalar'),2.5);
 check('removePrimDirect','/Output');
 assert.equal(call('getAttribute','/Output','width'),0);
 call('close'); console.log('USD object API WASM checks passed');
} finally { m.PThread?.terminateAllThreads(); }
