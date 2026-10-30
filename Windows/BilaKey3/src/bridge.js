'use strict';
// Offline line-oriented converter for the BilaKey3 Windows prototype.
const readline = require('node:readline');
const converter = require('./cvnss_converter.js');
process.stdout.write(JSON.stringify({ready:true,version:converter.VERSION,audit:converter.audit(),selfTest:converter.selfTest()})+'\n');
readline.createInterface({input:process.stdin,terminal:false}).on('line', line => {
  try {
    const request = JSON.parse(line);
    const words = (Array.isArray(request.words) ? request.words : []).slice(0,500);
    const result = words.map(value => {
      const word = String(value).slice(0,200);
      const cvss = converter.fromCvss(word);
      const cqn = converter.fromCqn(word);
      const inspect = converter.inspectWord(word,'cvss');
      return {word,fromCvss:cvss.cqn,fromCvssCode:cvss.cvss,fromCqnCvss:cqn.cvss,
        fromCqnCvn:cqn.cvn,ambiguous:inspect.ambiguous,critical:inspect.critical,
        candidates:inspect.candidates.slice(0,12)};
    });
    process.stdout.write(JSON.stringify({ok:true,words:result})+'\n');
  } catch (e) {
    process.stdout.write(JSON.stringify({ok:false,error:String(e.message||e)})+'\n');
  }
});
