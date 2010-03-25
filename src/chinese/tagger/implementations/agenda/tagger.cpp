/****************************************************************
 *                                                              *
 * agendachart.cpp - the agendachart tagger implementation.     *
 *                                                              *
 * The combination of agenda and rt                             *
 *                                                              *
 * Author: Yue Zhang                                            *
 *                                                              *
 * Computing Laboratory, Oxford. 2007.5                         *
 *                                                              *
 ****************************************************************/

#include "tagger.h"

using namespace chinese;
using namespace chinese::tagger;

static CWord g_emptyWord("");
static CTag g_beginTag(CTag::SENTENCE_BEGIN);

#define find_or_replace_word_cache(tmp_start, tmp_end) ( amount == 0 ? m_Cache.find(tmp_start, tmp_end, sentence) : m_Cache.replace(tmp_start, tmp_end, sentence) )

/*===============================================================
 *
 * CTagger - handles the features for the tagger
 *
 *==============================================================*/

/*---------------------------------------------------------------
 *
 * getOrUpdateSeparateScore - get or update the local score for a word in sentence
 *
 * When bigram is needed from the beginning of sentence, the
 * -BEGIN- tag and the empty word are used. 
 *
 * This implies that empty words should not be used in other 
 * situations. 
 *
 *--------------------------------------------------------------*/

SCORE_TYPE CTagger::getOrUpdateSeparateScore( const CStringVector *sentence, const CSubStateItem *item, unsigned long index, SCORE_TYPE amount, unsigned long round ) {
   static SCORE_TYPE nReturn ; 
   static unsigned long start_0; 
   static unsigned long start_1, end_1, length_1; 

   // about the words
   start_0 = item->getWordStart( index ) ;

   start_1 = index > 0 ? item->getWordStart( index-1 ) : 0 ;
   end_1 = index > 0 ? item->getWordEnd( index-1 ) : 0 ;
   assert(end_1 == start_0-1);
   length_1 = index > 0 ? item->getWordLength( index-1 ) : 0;

   start_2 = index > 1 ? item->getWordStart( index-2 ) : 0 ;
   end_2 = index > 1 ? item->getWordEnd( index-2 ) : 0 ;
   assert(end_2 == start_1-1);
   length_2 = index > 1 ? item->getWordLength( index-2 ) : 0;

   const CWord &word_1 = index>0 ? find_or_replace_word_cache( start_1, end_1 ) : g_emptyWord; 
   const CWord &word_2 = index>1 ? find_or_replace_word_cache( start_2, end_2 ) : g_emptyWord; 

   // about the length
   if( length_1 > LENGTH_MAX-1 ) length_1 = LENGTH_MAX-1 ;
   if( length_2 > LENGTH_MAX-1 ) length_2 = LENGTH_MAX-1 ;

   // about the chars
   const CWord &first_char_0 = find_or_replace_word_cache( start_0, start_0 );
   const CWord &first_char_1 = index>0 ? find_or_replace_word_cache( start_1, start_1 ) : g_emptyWord;

   const CWord &last_char_1 = index>0 ? find_or_replace_word_cache( end_1, end_1 ) : g_emptyWord;
   const CWord &last_char_2 = index>1 ? find_or_replace_word_cache( end_2, end_2 ) : g_emptyWord;
   const CWord &two_char = index>0 ? find_or_replace_word_cache( end_1, start_0 ) : g_emptyWord ;
   const CWord &word_1_first_char_0 = index>0 ? find_or_replace_word_cache( start_1, start_0 ) : g_emptyWord;
   const CWord &word_1_last_char_2 = index>1 ? find_or_replace_word_cache( end_2, end_1 ) : g_emptyWord;
   const CWord &three_char = ( length_1==1 && index>1 ) ? find_or_replace_word_cache( end_2, start_0 ) : g_emptyWord;

   static CTwoWords word_2_word_1, first_char_1_last_char_1, first_char_0_first_char_1, last_char_1_last_char_2 ;
   if (amount==0&&index>0) {
      word_2_word_1.refer( &word_1 , &word_2 ) ;
      first_char_1_last_char_1.refer( &first_char_1 , &last_char_1 ) ;
      first_char_0_first_char_1.refer( &first_char_0 , &first_char_1 ) ;
      last_char_1_last_char_2.refer( &last_char_1 , &last_char_2 ) ;
   }
   else {
      word_2_word_1.allocate( word_1, word_2 ) ;
      first_char_1_last_char_1.allocate( first_char_1, last_char_1 ) ;
      first_char_0_first_char_1.allocate( first_char_0, first_char_1 ) ;
      last_char_1_last_char_2.allocate( last_char_1, last_char2 ) ;
   }

   // about the tags 
   const CTag &tag_0 = item->getTag( index ) ;
   const CTag &tag_1 = index>0 ? item->getTag(index-1) : g_beginTag;
   const CTag &tag_2 = index>1 ? item->getTag(index-2) : g_beginTag;

   static CTaggedWord<CTag, TAG_SEPARATOR> wt1, wt2;
   static CTwoTaggedWords wt12;

   unsigned long long first_char_cat_0 = m_weights->m_mapCharTagDictionary.lookup(first_char_0) | (static_cast<unsigned long long>(1)<<tag_0.code()) ;
   unsigned long long last_char_cat_1 = m_weights->m_mapCharTagDictionary.lookup(last_char_1) | (static_cast<unsigned long long>(1)<<tag_1.code()) ;

   static int j ; 

   // adding scores with features
   if (index>0) {
      nReturn = m_weights->m_mapSeenWords.getOrUpdateScore( word_1 , m_nScoreIndex , amount , round ) ; 
      if (index>1) nReturn += m_weights->m_mapLastWordByWord.getOrUpdateScore( word_2_word_1 , m_nScoreIndex , amount , round ) ;
   }

   if ( length == 1 ) {
      nReturn += m_weights->m_mapOneCharWord.getOrUpdateScore( word , m_nScoreIndex , amount , round ) ;
   }
   else {
      nReturn += m_weights->m_mapFirstAndLastChars.getOrUpdateScore( first_char_1_last_char_1 , m_nScoreIndex , amount , round ) ;

      nReturn += m_weights->m_mapLengthByFirstChar.getOrUpdateScore( make_pair(first_char, length) , m_nScoreIndex , amount , round ) ;
      nReturn += m_weights->m_mapLengthByLastChar.getOrUpdateScore( make_pair(last_char, length) , m_nScoreIndex , amount , round ) ;
   }

   if ( index>0 ) {
      nReturn += m_weights->m_mapSeparateChars.getOrUpdateScore( two_char , m_nScoreIndex , amount , round ) ; 

      nReturn += m_weights->m_mapCurrentWordLastChar.getOrUpdateScore( currentword_lastchar , m_nScoreIndex , amount , round ) ;
      nReturn += m_weights->m_mapLastWordFirstChar.getOrUpdateScore( lastword_firstchar , m_nScoreIndex , amount , round ) ;

      nReturn += m_weights->m_mapFirstCharLastWordByWord.getOrUpdateScore( first_char_0_first_char_1 , m_nScoreIndex , amount , round ) ;
      nReturn += m_weights->m_mapLastWordByLastChar.getOrUpdateScore( last_char_1_last_char_2 , m_nScoreIndex , amount , round ) ;

      nReturn += m_weights->m_mapLengthByLastWord.getOrUpdateScore( make_pair(last_word, length) , m_nScoreIndex , amount , round ) ;
      nReturn += m_weights->m_mapLastLengthByWord.getOrUpdateScore( make_pair(word, last_length), m_nScoreIndex , amount , round ) ;
   }
  
   nReturn += m_weights->m_mapCurrentTag.getOrUpdateScore( make_pair(word_1, tag_1) , m_nScoreIndex , amount , round ) ; 
   if ( start > 0 ) {
      if ( last_length <= 2 ) nReturn += m_weights->m_mapTagByLastWord.getOrUpdateScore( make_pair(last_word, tag) , m_nScoreIndex , amount , round ) ;
      if ( length <= 2 ) nReturn += m_weights->m_mapLastTagByWord.getOrUpdateScore( make_pair(word, last_tag) , m_nScoreIndex , amount , round ) ;
      if ( length <= 2 ) nReturn += m_weights->m_mapTagByWordAndPrevChar.getOrUpdateScore( make_pair(currentword_lastchar, tag) , m_nScoreIndex , amount , round ) ;
      if ( last_length <= 2 ) nReturn += m_weights->m_mapTagByWordAndNextChar.getOrUpdateScore( make_pair(lastword_firstchar, last_tag) , m_nScoreIndex , amount , round ) ;
   }
   if ( length == 1 ) {
      if ( start > 0 && end < sentence->size()-1 )
         nReturn += m_weights->m_mapTagOfOneCharWord.getOrUpdateScore( make_pair(three_char, tag) , m_nScoreIndex , amount , round ) ;
   }
   else {
      nReturn += m_weights->m_mapTagByFirstChar.getOrUpdateScore( make_pair(first_char, tag) , m_nScoreIndex , amount , round ) ; 
      nReturn += m_weights->m_mapTagByLastChar.getOrUpdateScore( make_pair(last_char, tag) , m_nScoreIndex , amount , round ) ;
      nReturn += m_weights->m_mapTagByFirstCharCat.getOrUpdateScore( make_pair(first_char_cat, tag) , m_nScoreIndex , amount , round ) ; 
      nReturn += m_weights->m_mapTagByLastCharCat.getOrUpdateScore( make_pair(last_char_cat, tag) , m_nScoreIndex , amount , round ) ;

      nReturn += m_weights->m_mapTagByChar.getOrUpdateScore( 
                              make_pair( first_char, tag), 
                              m_nScoreIndex , amount , round ) ;

      for ( j = 0 ; j < word_length ; ++j ) {

         if ( j < word_length-1 ) {
            if (amount==0) {
               wt1.load( m_WordCache.find(start+j, start+j, sentence) , tag ); 
               wt2.load(last_char); 
               wt12.refer(&wt1, &wt2); 
            }
            else {
               wt1.load( m_WordCache.replace(start+j, start+j, sentence) , tag ); 
               wt2.load(last_char); 
               wt12.allocate(wt1, wt2); 
            }
            nReturn += m_weights->m_mapTaggedCharByLastChar.getOrUpdateScore(wt12, m_nScoreIndex, amount, round) ;
         }
      }
   }

   return nReturn;
}

/*---------------------------------------------------------------
 *
 * getOrUpdateAppendScore - get or update the local score for a word in sentence
 *
 * When bigram is needed from the beginning of sentence, the
 * -BEGIN- tag and the empty word are used. 
 *
 * This implies that empty words should not be used in other 
 * situations. 
 *
 *--------------------------------------------------------------*/

SCORE_TYPE CTagger::getOrUpdateAppendScore( const CStringVector *sentence, const CSubStateItem *item, unsigned long index, unsigned long char_index, SCORE_TYPE amount, unsigned long round ) {
   static SCORE_TYPE nReturn ; 
   assert(char_index>0);
   
   static unsigned long start;
   start = item->getWordStart( index ) ;

   const CWord &char_unigram = find_or_replace_word_cache( char_index, char_index );
   const CWord &char_bigram = find_or_replace_word_cache( char_index-1, char_index );

   const CWord &first_char = find_or_replace_word_cache( start, start );
   const CWord &prev_char = find_or_replace_word_cache( char_index-1, char_index-1 );

   // about the tags 
   const CTag &tag = item->getTag(index);
   const CTag &last_tag = index>0 ? item->getTag(index-1) : g_beginTag;
   const CTag &second_last_tag = index>1 ? item->getTag(index-2) : g_beginTag;

   static CTaggedWord<CTag, TAG_SEPARATOR> wt1, wt2;
   static CTwoTaggedWords wt12;

   static CTagSet<CTag, 2> tagset2;
   static CTagSet<CTag, 3> tagset3;

   static unsigned long tmp ; 

   // adding scores with features
   nReturn = 0;

   tmp = encodeTags(tag, last_tag);
   tagset2.load( tmp );
   nReturn += m_weights->m_mapLastTagByTag.getOrUpdateScore( tagset2, m_nScoreIndex , amount , round ) ;
   tagset3.load( encodeTags(tmp, second_last_tag);
   nReturn += m_weights->m_mapLastTwoTagsByTag.getOrUpdateScore( tagset3, m_nScoreIndex , amount , round ) ;

   nReturn += m_weights->m_mapTagByChar.getOrUpdateScore( make_pair(char_unigram, tag), m_nScoreIndex , amount , round ) ;

   wt1.load(char_unigram); 
   wt2.load(first_char);
   if (amount==0) { wt12.refer(&wt1, &wt2); } else { wt12.allocate(wt1, wt2); }
   nReturn += m_weights->m_mapTaggedCharByFirstChar.getOrUpdateScore( wt12, m_nScoreIndex, amount, round ) ;

   if (char_unigram == prev_char) 
      nReturn += m_weights->m_mapRepeatedCharByTag.getOrUpdateScore( make_pair(char_unigram, tag), m_nScoreIndex, amount, round) ;

   nReturn += m_weights->m_mapConsecutiveChars.getOrUpdateScore( char_bigram, m_nScoreIndex, amount, round ) ; 

   return nReturn;
}

/*---------------------------------------------------------------
 *
 * buildStateItem - builds item
 *
 * Inputs: the raw sentence in input format
 *         the tagged sentence in output format
 *         the state item which is retval
 *
 *--------------------------------------------------------------*/

inline void buildStateItem(const CStringVector *raw, const CTwoStringVector *tagged, CSubStateItem *item) {
   static int i, ri, rawlen, taggedlen;
   item->clear();
   // add each output word
   rawlen = 0; ri = 0; // raw index
   taggedlen = 0;
   for (i=0; i<tagged->size(); ++i) {
      taggedlen += tagged->at(i).first.size();
      while ( rawlen < taggedlen ) {
         rawlen += raw->at(ri).size();
         ++ri;
      }
      item->append(ri-1, CTag(tagged->at(i).second).code());
   }
}

/*---------------------------------------------------------------
 *
 * generate - helper function that generates tagged output
 *
 *--------------------------------------------------------------*/

void generate(const CSubStateItem *stateItem, CStringVector *sentence, CTagger *tagger, CTwoStringVector *vReturn) {
   string s;
   for (int j=0; j<stateItem->size(); ++j) { 
      s.clear();
      for (int k=stateItem->getWordStart(j); k<stateItem->getWordEnd(j)+1; ++k) {
         assert(sentence->at(k)!=" "); //[--SPACE--]
         s += sentence->at(k);
      }
      vReturn->push_back(make_pair(s, stateItem->getTag(j).str()));
   }
}

/*---------------------------------------------------------------
 *
 * train - train the module auto
 *
 * Since we rely on external trainer, this method is empty
 *
 *--------------------------------------------------------------*/

bool CTagger::train( const CStringVector * sentence , const CTwoStringVector * correct) {
   ++m_nTrainingRound ;
   buildStateItem( sentence, correct, &m_goldState);
   // Updates that are common for all example
   for ( unsigned i=0; i<correct->size(); ++i ) {

      const CWord &word = correct->at(i).first ;
      unsigned long tag = CTag( correct->at(i).second ).code() ;

      static CStringVector chars;
      chars.clear(); 
      getCharactersFromUTF8String(correct->at(i).first, &chars);

      static string part_word;
      part_word.clear();

      m_weights->m_mapWordFrequency[word]++;
      if (m_weights->m_mapWordFrequency[word]>m_weights->m_nMaxWordFrequency) 
         m_weights->m_nMaxWordFrequency = m_weights->m_mapWordFrequency[word];

      m_weights->m_mapTagDictionary.add(word, tag);
      for ( unsigned j=0 ; j<chars.size() ; ++j ) {
         m_weights->m_mapCharTagDictionary.add(chars[j], tag) ;
         if ( PENN_TAG_CLOSED[tag] ) {
            part_word += chars[j];
            m_weights->m_mapCanStart.add(part_word, tag);
         }
      }

      if ( !m_weights->m_Knowledge ||
          (!m_weights->m_Knowledge->isFWorCD(chars[0])&&
           !m_weights->m_Knowledge->isFWorCD(chars[chars.size()-1])))
         m_weights->setMaxLengthByTag( tag , chars.size() ) ;
   }
   tag( sentence, NULL, NULL, 1, NULL );
   return m_bTrainingError;
}

/*---------------------------------------------------------------
 *
 * tag - assign POS tags to a sentence
 *
 * Returns: makes a new instance of CTwoStringVector 
 *
 *--------------------------------------------------------------*/

void CTagger::tag( const CStringVector * sentence_input , CTwoStringVector * vReturn , SCORE_TYPE * out_scores , unsigned long nBest , const CBitArray * prunes ) {
   clock_t total_start_time = clock();;
   int temp_index;
   const CSubStateItem *pGenerator;
   CSubStateItem tempState;
   int j, k;
   unsigned tag;
   unsigned index, last_tag;
 
   static CSubStateItem uniqueItems[AGENDA_SIZE];
   unsigned uniqueIndex;

   static CStringVector sentence;
   static CRule rules(m_weights->m_bSegmentationRules);
   rules.segment(sentence_input, &sentence);
   const unsigned length=sentence.size();

   static CSubStateItem goldState;
   goldState.clear();

   TRACE("Initialising the tagging process...");
   m_WordCache.clear(); 
   tempState.clear();
   m_Agenda.clear();
   m_Agenda.pushCandidate(&tempState);                   
   m_Agenda.nextRound();

   TRACE("Tagging started");
   //TRACE("initialisation time: " << clock() - start_time);
   for (index=0; index<length; index++) {

      // decide correction
      if ( m_bTrain ) {
         static bool bAnyCorrect;
         bAnyCorrect = false;
         pGenerator = m_Agenda.generatorStart();
         for (j=0; j<m_Agenda.generatorSize(); ++j) {
            if ( *pGenerator == goldState ) bAnyCorrect = true;
            pGenerator = m_Agenda.generatorNext();  // next generator
         }
         if ( !bAnyCorrect ) {
            TRACE("Training error at character " << index);
            pGenerator = m_Agenda.bestGenerator();
            updateScoreForState(&sentence, pGenerator, -1);
            updateScoreForState(&sentence, &goldState, 1);
            m_bTrainingError = true;
            return;
         }
      }


      // 2. generate by replacing items
      pGenerator = m_Agenda.generatorStart();
      for (j=0; j<m_Agenda.generatorSize(); ++j) {
         if ( ( index > 0 ) && ( rules.canAppend(index) ) && 
              pGenerator->getWordLength(pGenerator->size()-1) < 
                 m_weights->m_maxLengthByTag[pGenerator->getTag(pGenerator->size()-1).code()]
            ) {
            tempState.copy(pGenerator);
            tempState.replaceIndex(index);
            getOrUpdateAppendScore(&sentence, &tempState, tempState.size()-1, index);
            m_Agenda.pushCandidate(&tempState);
         } // if
         pGenerator = m_Agenda.generatorNext();  // next generator
      }

   //_
   // 1. generate new items according to each previous item. 
   // iterate postags
      for (tag=CTag::FIRST; tag<CTag::COUNT; ++tag) {

         pGenerator = m_Agenda.generatorStart();
         uniqueIndex=0;

         for (j=0; j<m_Agenda.generatorSize(); ++j) {

            if ( rules.canSeparate( index ) && 
                (index == 0 || canAssignTag( m_WordCache.find( pGenerator->getWordStart(pGenerator->size()-1), index-1, &sentence ), pGenerator->getTag(pGenerator->size()-1).code() )) && // last word
                 canStartWord(sentence, tag, index) // word
               ) {  
               tempState.copy(pGenerator);
               tempState.append(index, tag);
               tempState.score += getOrUpdateSeparateScore(&sentence, &tempState, tempState.size()-1);
               if (nBest==1) {
                  // make sure only the best is stored among all ending with the same pos
                  // bigram and last word (which is currently a single character)
                  bool bDuplicate = false;
                  for (temp_index=0; temp_index<uniqueIndex; ++temp_index) {
                     if (uniqueItems[temp_index].size() > 1 && 
                         tempState.size() > 1 && 
                         uniqueItems[temp_index].getTag(uniqueItems[temp_index].size()-2) == tempState.getTag(tempState.size()-2) 
                        ) {
                          bDuplicate = true;
                          if (uniqueItems[temp_index].score < tempState.score) {
                             uniqueItems[temp_index].copy(&tempState);
                          }
                          break;
                     }
                  }
                  if (!bDuplicate) {
                     // the number of generators is surely fewer than list size
                     uniqueItems[uniqueIndex++].copy(&tempState);
                  }
               }
               else {
                  m_Agenda.pushCandidate(&tempState);
               }
            }
            pGenerator = m_Agenda.generatorNext();  // next generator
         }
         // push candidates
         if (nBest == 1) {
            for (temp_index=0; temp_index<uniqueIndex; ++temp_index) {
               m_Agenda.pushCandidate(&uniqueItems[temp_index]);
            }
         }
      }//tag

      m_Agenda.nextRound(); // move round
      if (m_bTrain) goldState.follow(m_goldState);
   }
   
   if ( m_bTrain && 1 ) {
      pGenerator = m_Agenda.bestGenerator();
      if ( *pGenerator != goldState ) {
         TRACE("Training error at the last word");
         updateScoreForState(&sentence, pGenerator, -1);
         updateScoreForState(&sentence, &goldState, 1);
         m_bTrainingError = true;
      }
      m_bTrainingError = false;
      return;
   }
   TRACE("Outputing sentence");
   vReturn->clear();
   if (nBest == 1) {
      generate( m_Agenda.bestGenerator() , &sentence , this , vReturn ) ; 
      if (out_scores) out_scores[ 0 ] = m_Agenda.bestGenerator( )->score ;
   }
   else {
      m_Agenda.sortGenerators();
      for ( temp_index = 0 ; temp_index < nBest ; ++ temp_index ) {
         vReturn[ temp_index ].clear() ; 
         if (out_scores) out_scores[ temp_index ] = 0 ;
         if ( temp_index < m_Agenda.generatorSize() ) {
            generate( m_Agenda.generator( temp_index ) , &sentence , this , &(vReturn[ temp_index ]) ) ; 
            if (out_scores) out_scores[ temp_index ] = m_Agenda.bestGenerator( )->score ;
         }
      }
   }
   TRACE("Done, the highest score is: " << m_Agenda.bestGenerator()->score) ;
   TRACE("The total time spent: " << double(clock() - total_start_time)/CLOCKS_PER_SEC) ;
}




