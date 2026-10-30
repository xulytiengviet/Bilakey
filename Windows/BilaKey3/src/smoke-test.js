'use strict';
const c=require('./cvnss_converter.js');
const t=c.selfTest();if(!t.ok){console.error(t);process.exit(1);}
const cases=[['tôi','toiy'],['yêu','iwy'],['tiếng','tizb'],['việt','vidf']];
for(const [word,code] of cases) {
  const actual=c.fromCqn(word).cvss;
  if(actual!==code){console.error(word,actual,code);process.exit(1);}
}
console.log('CVNSS converter selfTest OK; 4 smoke tests OK; ambiguity groups:',c.audit().ambiguityGroups);
